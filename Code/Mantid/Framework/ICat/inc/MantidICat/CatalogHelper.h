#ifndef MANTID_ICAT_CATALOGHELPER_H_
#define MANTID_ICAT_CATALOGHELPER_H_

#include "MantidAPI/CatalogSession.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"

namespace Mantid
{
  namespace ICat
  {
    class CatalogHelper
    {
      public:

        CatalogHelper();

        // Helper method that formats a given timestamp.
        std::string formatDateTime(const time_t &timestamp, const std::string &format);

        /**
         * Save data value to table workspace if it exists, otherwise insert empty string.
         * @param value :: Pointer to input value.
         * @param table :: Table row reference.
         */
        template<class T>
        void saveValueToTableWorkspace(T* value, API::TableRow &table)
        {
          if(value || value != 0) table << *value;
          else table << "";
        }

        /**
         * Obtains related datafile information from the datafiles container passed in,
         * and saves information for each datafile to a workspace.
         * @param datafiles :: The holder containing datafiles.
         * @param outputws  :: The workspace to write the datafile information to.
         */
        template <typename T>
        void saveDataFiles(T& datafiles, API::ITableWorkspace_sptr& outputws)
        {
          if (outputws->getColumnNames().empty())
          {
            outputws->addColumn("str","Name");
            outputws->addColumn("str","Location");
            outputws->addColumn("str","Create Time");
            outputws->addColumn("long64","Id");
            outputws->addColumn("long64","File size(bytes)");
            outputws->addColumn("str","File size");
            outputws->addColumn("str","Description");
          }

          for(auto iter = datafiles.begin(); iter != datafiles.end(); ++iter)
          {
            API::TableRow table = outputws->appendRow();

            saveValueToTableWorkspace((*iter)->name, table);
            saveValueToTableWorkspace((*iter)->location, table);

            std::string createDate = formatDateTime(*(*iter)->createTime, "%Y-%m-%d %H:%M:%S");
            saveValueToTableWorkspace(&createDate, table);

            saveValueToTableWorkspace((*iter)->id, table);
            saveValueToTableWorkspace((*iter)->fileSize, table);

            std::string fileSize = bytesToString(*(*iter)->fileSize);
            saveValueToTableWorkspace(&fileSize, table);

            saveValueToTableWorkspace((*iter)->description, table);
          }
        }

        /**
         * Parse the message returned by ICAT to obtain a user friendly error message.
         * @param icatProxy :: The PortBindingProxy of the ICAT instance.
         */
        template <class T>
        void throwErrorMessage(T& icatProxy)
        {
          char buf[600];
          icatProxy.soap_sprint_fault(buf,600);
          std::string error(buf);

          std::string begmsg("<message>");
          std::string endmsg("</message>");

          auto start = error.find(begmsg);
          auto end   = error.find(endmsg);
          std::string exception;

          if(start != std::string::npos && end != std::string::npos)
          {
            exception = error.substr(start + begmsg.length(), end - (start + begmsg.length()));
          }

          throw std::runtime_error(exception);
        }

        /**
         * Casts the container's content types from A to B.
         * @param container :: The container holding objects of type A.
         * @return A vector containing objects of type B.
         */
        template <typename A, class B>
        std::vector<B> castContainerType(A& container,B&)
        {
          std::vector<B> data;

          for(auto iter = container.begin(); iter != container.end(); ++iter)
          {
            B desiredType = dynamic_cast<B>(*iter);
            if (desiredType) data.push_back(desiredType);
            else throw std::runtime_error("An error occurred casting vector types.");
          }
          return data;
        }

        /**
         * Obtains information for investigations in the container.
         * @param investigations :: The holder containing investigations.
         * @param outputws :: Shared pointer to output workspace.
         */
        template <typename T>
        void saveInvestigations(T& investigations, API::ITableWorkspace_sptr& outputws)
        {
          if (outputws->getColumnNames().empty())
          {
            outputws->addColumn("str","Investigation id");
            outputws->addColumn("str","Facility");
            outputws->addColumn("str","Title");
            outputws->addColumn("str","Instrument");
            outputws->addColumn("str","Run range");
            outputws->addColumn("str","Start date");
            outputws->addColumn("str","End date");
            outputws->addColumn("str","SessionID");
          }

          for(auto iter = investigations.begin(); iter != investigations.end(); ++iter)
          {
            API::TableRow table = outputws->appendRow();
            // Used to insert an empty string into the cell if value does not exist.
            std::string emptyCell("");

            // Now add the relevant investigation data to the table (They always exist).
            saveValueToTableWorkspace((*iter)->name, table);
            saveValueToTableWorkspace((*iter)->facility->name, table);
            saveValueToTableWorkspace((*iter)->title, table);
            saveValueToTableWorkspace((*iter)->investigationInstruments.at(0)->instrument->name, table);

            // Verify that the run parameters vector exist prior to doing anything.
            // Since some investigations may not have run parameters.
            if (!(*iter)->parameters.empty())
            {
              saveValueToTableWorkspace((*iter)->parameters[0]->stringValue, table);
            }
            else
            {
             saveValueToTableWorkspace(&emptyCell, table);
            }

            // Again, we need to check first if start and end date exist prior to insertion.
            if ((*iter)->startDate)
            {
             std::string startDate = formatDateTime(*(*iter)->startDate, "%Y-%m-%d");
             saveValueToTableWorkspace(&startDate, table);
            }
            else
            {
             saveValueToTableWorkspace(&emptyCell, table);
            }

            if ((*iter)->endDate)
            {
             std::string endDate = formatDateTime(*(*iter)->endDate, "%Y-%m-%d");
             saveValueToTableWorkspace(&endDate, table);
            }
            else
            {
             saveValueToTableWorkspace(&emptyCell, table);
            }

            std::string sessionID = session->getSessionId();
            saveValueToTableWorkspace(&sessionID, table);
          }
        }

        /**
         * Saves information of datasets to a workspace.
         * @param datasets :: The holder containing datasets.
         * @param outputws :: The workspace to write the datasets information to.
         */
        template <typename T>
        void saveDataSets(T& datasets, API::ITableWorkspace_sptr& outputws)
        {
          if (outputws->getColumnNames().empty())
          {
            // Add rows headers to the output workspace.
            outputws->addColumn("str","Name");
            outputws->addColumn("str","Description");
            outputws->addColumn("str","Start date");
            outputws->addColumn("str","End date");
            outputws->addColumn("str","DOI");
          }

          std::string temp("");

          for(auto iter = datasets.begin(); iter != datasets.end(); ++iter)
          {
            API::TableRow table = outputws->appendRow();

            saveValueToTableWorkspace(&temp, table);
            saveValueToTableWorkspace(&temp, table);
            saveValueToTableWorkspace(&temp, table);
            saveValueToTableWorkspace(&temp, table);
            saveValueToTableWorkspace(&temp, table);
          }
        }

        // Stores the session details for a specific catalog.
        API::CatalogSession_sptr session;

      private:
        // Convert a file size to human readable file format.
        std::string bytesToString(int64_t &fileSize);
    };
  }
}

#endif //MANTID_ICAT_CATALOGHELPER_H_
