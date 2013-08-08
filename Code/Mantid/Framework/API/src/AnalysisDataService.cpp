#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidKernel/Strings.h"

namespace Mantid
{
  namespace API
  {

    //-------------------------------------------------------------------------
    // Nested class methods
    //-------------------------------------------------------------------------
    /**
     * Constructor.
     * @param name :: The name of a workspace group.
     */
    AnalysisDataServiceImpl::GroupUpdatedNotification::GroupUpdatedNotification(const std::string& name) : 
      DataServiceNotification( name, AnalysisDataService::Instance().retrieve( name ) )
    {
    }
    /**
     * Returns the workspace pointer cast to WorkspaceGroup
     */
    boost::shared_ptr<const WorkspaceGroup> AnalysisDataServiceImpl::GroupUpdatedNotification::getWorkspaceGroup() const
    {
      return boost::dynamic_pointer_cast<const WorkspaceGroup>( this->object() );
    }

    //-------------------------------------------------------------------------
    // Public methods
    //-------------------------------------------------------------------------
    /**
    * Is the given name a valid name for an object in the ADS
    * @param name A string containing a possible name for an object in the ADS
    * @return An empty string if the name is valid or an error message stating the problem 
    * if the name is unacceptable.
    */
    const std::string AnalysisDataServiceImpl::isValid(const std::string & name) const
    {
      std::string error("");
      const std::string & illegal = illegalCharacters();
      if( illegal.empty() ) return error; //Quick route out.
      const size_t length = name.size();
      for(size_t i = 0; i < length; ++i)
      {
        if( illegal.find_first_of(name[i]) != std::string::npos )
        {
          std::ostringstream strm;
          strm << "Invalid object name '" << name  << "'. Names cannot contain any of the following characters: " << illegal;
          error = strm.str();
          break;
        }
      }
      return error;
    }

    /**
     * Overridden add member to attach the name to the workspace when a workspace object is added to the service
     * If the name already exists then this throws a std::runtime_error. If a workspace group is added adds the
     * members which are not in the ADS yet.
     * @param name The name of the object
     * @param workspace The shared pointer to the workspace to store
     */
    void AnalysisDataServiceImpl::add( const std::string& name, const boost::shared_ptr<API::Workspace>& workspace)
    {
      const bool replaceWS(false);
      addToService(name, workspace, replaceWS);
    }

   /**
     * Overridden addOrReplace member to attach the name to the workspace when a workspace object is added to the service.
     * This will overwrite one of the same name. If the workspace is group adds or replaces its members.
     * @param name The name of the object
     * @param workspace The shared pointer to the workspace to store
     */
    void AnalysisDataServiceImpl::addOrReplace( const std::string& name, const boost::shared_ptr<API::Workspace>& workspace)
    {
      const bool replaceWS(true);
      addToService(name, workspace, replaceWS);
    }

    /**
     * Add a workspace to a group. The group and the workspace must be in the ADS.
     * @param groupName :: A group name.
     * @param wsName :: Name of a workspace to add to the group.
     */
    void AnalysisDataServiceImpl::addToGroup(const std::string &groupName, const std::string &wsName)
    {
        WorkspaceGroup_sptr group = retrieveWS<WorkspaceGroup>( groupName );
        if ( !group )
        {
            throw std::runtime_error("Workspace " + groupName + " is not a workspace group.");
        }
        auto ws = retrieve( wsName );
        group->addWorkspace( ws );
        notificationCenter.postNotification(new GroupUpdatedNotification( groupName ));
    }

    /**
     * Remove a workspace group and all its members from the ADS.
     * @param name :: A group to remove.
     */
    void AnalysisDataServiceImpl::deepRemoveGroup(const std::string &name)
    {
        WorkspaceGroup_sptr group = retrieveWS<WorkspaceGroup>( name );
        if ( !group )
        {
            throw std::runtime_error("Workspace " + name + " is not a workspace group.");
        }
        group->observeADSNotifications( false );
        for(size_t i = 0; i < group->size(); ++i)
        {
            auto ws = group->getItem(i);
            WorkspaceGroup_sptr gws = boost::dynamic_pointer_cast<WorkspaceGroup>( ws );
            if ( gws )
            {
                // if a member is a group remove its items as well
                deepRemoveGroup( gws->name() );
            }
            else
            {
                remove( ws->name() );
            }
        }
        remove( name );
    }

    /**
     * Remove a workspace from a group but not from the ADS.
     *
     * @param groupName :: Name of a workspace group.
     * @param wsName :: Name of a workspace to remove.
     */
    void AnalysisDataServiceImpl::removeFromGroup(const std::string &groupName, const std::string &wsName)
    {
        WorkspaceGroup_sptr group = retrieveWS<WorkspaceGroup>( groupName );
        if ( !group )
        {
            throw std::runtime_error("Workspace " + groupName + " is not a workspace group.");
        }
        if ( !group->contains(wsName) )
        {
            throw std::runtime_error("WorkspaceGroup " + groupName + " does not containt workspace " + wsName);
        }
        group->removeByADS( wsName );
        notificationCenter.postNotification(new GroupUpdatedNotification( groupName ));
    }

    /**
     * @return A pointer to the root node of the info tree.
     */
    Workspace::InfoNode *AnalysisDataServiceImpl::createInfoTree() const
    {
        auto workspaces = getObjects();

        // collect all groups and put them into temporary rootGroup
        WorkspaceGroup rootGroup;
        for( auto ws = workspaces.begin(); ws != workspaces.end(); ++ws )
        {
            WorkspaceGroup_sptr group = boost::dynamic_pointer_cast<WorkspaceGroup>( *ws );
            if ( group )
            {
                rootGroup.addWorkspace( group );
            }
        }

        // build the tree
        Workspace::InfoNode *root = new Workspace::InfoNode;
        for( auto ws = workspaces.begin(); ws != workspaces.end(); ++ws )
        {
            if ( !rootGroup.isInChildGroup(**ws) )
            {
                (**ws).addInfoNodeTo( *root );
            }
        }
        return root;
    }

    //-------------------------------------------------------------------------
    // Private methods
    //-------------------------------------------------------------------------

    /**
     * Constructor
     */
    AnalysisDataServiceImpl::AnalysisDataServiceImpl()
      :Mantid::Kernel::DataService<Mantid::API::Workspace>("AnalysisDataService"), m_illegalChars()
    {
    }

    /**
     * Destructor
     */
    AnalysisDataServiceImpl::~AnalysisDataServiceImpl()
    {

    }


    // The following is commented using /// rather than /** to stop the compiler complaining
    // about the special characters in the comment fields.
    /// Return a string containing the characters not allowed in names objects within ADS
    /// @returns A n array of c strings containing the following characters: " +-/*\%<>&|^~=!@()[]{},:.`$?"
    const std::string & AnalysisDataServiceImpl::illegalCharacters() const
    {
      return m_illegalChars;
    }

    /**
     * Set the list of illegal characeters
     * @param illegalChars A string containing the characters, as one long string, that are not to be accepted by the ADS
     * NOTE: This only affects further additions to the ADS
     */
    void AnalysisDataServiceImpl::setIllegalCharacterList(const std::string & illegalChars)
    {
      m_illegalChars = illegalChars;
    }


    /**
     * Checks the name is valid
     * @param name A string containing the name to check. If the name is invalid a std::invalid_argument is thrown
     */
    void AnalysisDataServiceImpl::verifyName(const std::string & name)
    {
      const std::string error = isValid(name);
      if( !error.empty() )
      {
        throw std::invalid_argument(error);
      }
    }

    /**
     * If replace=false, a group is being added & member already exists in the service then this will throw and that member will have no name
     * @param name The name to attach to the workspace
     * @param workspace The pointer to be added
     * @param replace If true then addOrReplace is called, else just add
     */
    void AnalysisDataServiceImpl::addToService(const std::string & name, const boost::shared_ptr<API::Workspace> & workspace,
                                               const bool replace)
    {
      verifyName(name);
      if(replace) Kernel::DataService<API::Workspace>::addOrReplace(name, workspace);
      else Kernel::DataService<API::Workspace>::add(name, workspace);

      // if a group is added, turn on ADS observers
      if( auto group = boost::dynamic_pointer_cast<WorkspaceGroup>(workspace) )
      {
        group->observeADSNotifications(true);
      }
    }

  } // Namespace API
} // Namespace Mantid

