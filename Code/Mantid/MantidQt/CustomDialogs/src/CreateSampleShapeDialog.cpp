//---------------------------
// Includes
//--------------------------
#include "MantidQtCustomDialogs/CreateSampleShapeDialog.h"
#include "MantidQtAPI/AlgorithmInputHistory.h"
#include "MantidQtCustomDialogs/SampleShapeHelpers.h"
#include "MantidQtCustomDialogs/MantidGLWidget.h"
#include "MantidAPI/AlgorithmManager.h"

#include "MantidGeometry/Objects/ShapeFactory.h"
#include "MantidGeometry/Objects/Object.h"

#include <Poco/DOM/Document.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/Path.h>

#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QShortcut>
#include <QCloseEvent>

#include <iostream>
#include <exception>

//Add this class to the list of specialised dialogs in this namespace
namespace MantidQt
{
namespace CustomDialogs
{
  DECLARE_DIALOG(CreateSampleShapeDialog);
}
}

// Just to save writing this everywhere 
using namespace MantidQt::CustomDialogs;
using namespace Mantid::API;
using namespace Mantid::Geometry;
using namespace Mantid::Kernel;
using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::Element;
using Poco::XML::Node;
using Poco::XML::NodeList;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::NamedNodeMap;
//---------------------------------------
// Public member functions
//---------------------------------------
/**
 * Constructor
 */
CreateSampleShapeDialog::CreateSampleShapeDialog(QWidget *parent) :
  AlgorithmDialog(parent), m_setup_map(), m_details_map(), m_ops_map()
{
  m_object_viewer = new MantidGLWidget;
}

/**
 * Destructor
 */
CreateSampleShapeDialog::~CreateSampleShapeDialog()
{
  // Delete the objects created. The shape details static counter is decremented when
  // its destructor is called
  QMutableMapIterator<BinaryTreeWidgetItem*, ShapeDetails*> itr(m_details_map);
  while( itr.hasNext() ) 
  {
    itr.next();
    ShapeDetails* obj = itr.value();
    itr.remove();
    delete obj;
  }
  QMutableMapIterator<BinaryTreeWidgetItem*, Operation*> itrb(m_ops_map);
  while( itrb.hasNext() ) 
  {
    itrb.next();
    Operation *obj = itrb.value();
    itrb.remove();
    delete obj;
  }
}

/**
 * Set up the dialog
 */
void CreateSampleShapeDialog::initLayout()
{
  //The main setup function
  m_uiForm.setupUi(this);
  
  // Create the map of instantiaters. The keys defined here are used to generate the shape 
  // menu items
  m_setup_map.clear();
  m_setup_map["sphere"] = new ShapeDetailsInstantiator<SphereDetails>;
  m_setup_map["cylinder"] = new ShapeDetailsInstantiator<CylinderDetails>;
  m_setup_map["infinite cylinder"] = new ShapeDetailsInstantiator<InfiniteCylinderDetails>();
  m_setup_map["cylinder ring slice"] = new ShapeDetailsInstantiator<SliceOfCylinderRingDetails>();
  m_setup_map["cone"] = new ShapeDetailsInstantiator<ConeDetails>();
  m_setup_map["infinite cone"] = new ShapeDetailsInstantiator<InfiniteConeDetails>();
  m_setup_map["infinite plane"] = new ShapeDetailsInstantiator<InfinitePlaneDetails>();
  m_setup_map["cuboid"] = new ShapeDetailsInstantiator<CuboidDetails>();
  m_setup_map["hexahedron"] = new ShapeDetailsInstantiator<HexahedronDetails>();
  //  m_setup_map["torus"] = new ShapeDetailsInstantiator<TorusDetails>();

  //The binary tree
  m_shapeTree = new BinaryTreeWidget(this);
  m_shapeTree->setColumnCount(1);
  m_shapeTree->setHeaderLabel("");
  m_shapeTree->setContextMenuPolicy(Qt::CustomContextMenu);
  m_shapeTree->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_shapeTree->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(m_shapeTree, SIGNAL(customContextMenuRequested(const QPoint &)), 
	  this, SLOT(handleTreeContextMenuRequest(const QPoint &)));
  connect(m_shapeTree, SIGNAL(itemSelectionChanged()), this, SLOT(setupDetailsBox()));
  connect(m_shapeTree, SIGNAL(treeDataChange(BinaryTreeWidgetItem*, int)), this, 
	  SLOT(changeTreeData(BinaryTreeWidgetItem*, int)));

  QPushButton *view_shape_btn = new QPushButton("Update 3D view");
  connect(view_shape_btn, SIGNAL(clicked()), this, SLOT(update3DView()));
  QHBoxLayout *bottom = new QHBoxLayout;
  bottom->addWidget(view_shape_btn);
  bottom->addStretch();

  //Shape box layout
  QVBoxLayout *shape_box_layout = new QVBoxLayout;
  shape_box_layout->addWidget(m_shapeTree);
  shape_box_layout->addLayout(bottom);
  m_uiForm.shape_box->setLayout(shape_box_layout);
  
  QShortcut *delete_key = new QShortcut(QKeySequence(Qt::Key_Delete), this);
  connect(delete_key, SIGNAL(activated()), this, SLOT(handleDeleteRequest()));

  QVBoxLayout *view_box_layout = new QVBoxLayout;
  view_box_layout->addWidget(m_object_viewer);
  m_uiForm.view_box->setLayout(view_box_layout);

  // Check input workspace property. If there are available workspaces then
  // these have been set as allowed values
  std::set<std::string> workspaces = getAlgorithmProperty("InputWorkspace")->allowedValues();
  for( std::set<std::string>::const_iterator itr = workspaces.begin(); itr != workspaces.end(); ++itr )
  {
    m_uiForm.wksp_opt->addItem(QString::fromStdString(*itr));
  }

  tie(m_uiForm.wksp_opt, "InputWorkspace", m_uiForm.bottomlayout);

  //Connect the help button
  connect(m_uiForm.helpButton, SIGNAL(clicked()), this, SLOT(helpClicked()));
  
  //populate the tree with the selected workspace's shape data if possible
  populateTree(m_uiForm.wksp_opt->currentText().toStdString());
}

void CreateSampleShapeDialog::populateTree(const std::string &workspace)
{
  auto ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(workspace);
  const Object * shape = &(ws->sample().getShape());
  if (shape->hasValidShape())
  {

    //parse the XML and get the shapes from it

    //Get the XML
    std::string shapeXML = shape->getShapeXML();

    DOMParser pParser;
    try
    {
      m_pDoc = pParser.parseString(shapeXML);
    }
    catch(Poco::Exception& exc)
    {
      throw std::runtime_error(exc.displayText() + ". Unable to parse shape XML");
    }
    catch(...)
    {
      throw std::runtime_error("Unable to parse shape XML");
    }
    //get the root element
    m_pRootElem = m_pDoc->documentElement();
    if ( !m_pRootElem->hasChildNodes() )
    {
      throw std::runtime_error("No root element in XML");
    }
    //get the algebra equasion ready to parse the tree from
    NodeList* algebraNL = m_pRootElem->getElementsByTagName("algebra");
    if ( algebraNL->length() == 0 )
    {
      throw std::runtime_error("No shape algebra definiton");
    }
    NamedNodeMap* alg = algebraNL->item(0)->attributes();
    algebraNL->release();
    Node* algVal = alg->getNamedItem("val");
    alg->release();
    QString equation(algVal->getNodeValue().c_str());
    if (equation == "")
    {
      throw std::runtime_error("No shape algebra definition");
    }

    int rootTreeNodeKey = parseEquation(&equation);
    
  }
}

int CreateSampleShapeDialog::parseEquation(QString* eq)
{
  
    //Parse the algebra function looking for [space], #, :, (, and )
    int operations = eq->count(QRegExp("[ :]"));
    if (operations == 0)
    {
      //there was only a shape or a complement of a shape, so we can deal with that as is, no messing about required or at the very most i have to remove some parentheses
      //strip out any stray parentheses
      eq->remove(QChar('('));
      eq->remove(QChar(')'));
      //check if the complement - it would contian a #
      int hashes = eq->count(QChar('#'));
      if (hashes == 1)
      {
      }
      else if (hashes > 1 || hashes < 0)
      {
        //if there wasn't jsut one hash or there was somethign wierd back from count, throw

      }
      else
      {
        //there weren't any hashes, create an antry for a normal shape
      }
    }
    else if (operations == 1)
    {
      //there is only one opperation there, so that's the parent at this level
      //split it at either side and pass it on
      //there's no need to mess about with parentheses as these are single opperands and the above case will deal with them
      QString eqL;
      QString eqR;
      int opPos = eq->indexOf(QRegExp("[ :]"));
      eqL = eq->left(opPos);
      eqR = eq->right(eq->length() - (opPos + 1));
      int leftInd = parseEquation(&eqL);
      int rightInd = parseEquation(&eqR);
      //add the info to the vector in the format [0] = L, [1] = shape/operation, [2] = R
      //then add that to the map

    }
    else
    {
      //there is more than one operation, i need to find the central one in order to split this further

      //i could find this out a different way by itterating ove the entire string incrementing a counter for each open and decrementing for each close. it should be >0 if there are more opens, <0 if there are mroe closes and 0 if they're equal
      int openBrackets = eq->count(QChar('('));
      int closeBrackets = eq->count(QChar(')'));
      if (openBrackets > closeBrackets)
      {
        //there's an unpaired open bracket, persumably at the start, left from splitting
      }
      else if (openBrackets < closeBrackets)
      {
        //there's an unpaired close bracket, persumably at the end, left from splitting

      }
      else
      {
        //there is an equal amount of opens and close brackets so it's assumed that they match up
        //count opens until the first close, then count the same number of closes, the central operator should be the next character 
      }
      /*
      //look at position 1 on the string, if it's not a ( or # then the first operand is most likely a straight shape, so look for the operator
      if (eq->at(0) == QChar('(') && eq->at(eq->length()- 1) == QChar(')'))
      {
        //the equasion is surronded in brackets but check to see if they match
        int openBrackets = eq->count(QChar('('));

      }*/
    }

    

    int u = 6;
}

/**
 * Retrieve the input from the dialog
 */
void CreateSampleShapeDialog::parseInput()
{
  QString xml = constructShapeXML();

  if( m_shapeTree->topLevelItemCount() > 0 && xml.isEmpty() )
  {
    QMessageBox::information(this, "CreateSampleShapeDialog", 
			     "An error occurred while parsing the shape tree.\n"
			     "Please check that each node has two children and the lowest elements are primitive shapes.");
    return;
  }

  storePropertyValue("ShapeXML", xml);
    
  // Get workspace value
  storePropertyValue("InputWorkspace", m_uiForm.wksp_opt->currentText());
}

/**
 * Validates all the shapes in the 3d space to see if they have enough information to render
 * @return Boolean : true if successful, false if one or more shapes failed to validate.
 */
bool CreateSampleShapeDialog::validate() const
{
  //Look through all the shapes to check if they all have details
  QMapIterator<BinaryTreeWidgetItem*, ShapeDetails*> itr(m_details_map);
  while (itr.hasNext())
  {
    itr.next();
    if (!itr.value()->valid())
    {
      return false;
    }
  }
  return true;
}
/**
 * A slot that can be used to connect a button that accepts the dialog if
 * all of the properties are valid
 */
void CreateSampleShapeDialog::accept()
{
  //Specific validation for the shapes
  if (validate())
  {
    // Get property values
    parse();
  
    //Try and set and validate the properties and 
    if( setPropertyValues() )
    {
      //Store input for next time
      saveInput();
      QDialog::accept();
    }
    else
    {
      QMessageBox::critical(this, "", 
              "One or more shapes are invalid so the model cannot be rendered.\n"
              "Please check all shapes in the model for any missing or invalid data." );
    }
  }
  else
  {
    QMessageBox::critical(this, "", 
              "One or more shapes are invalid so the model cannot be rendered.\n"
              "Please check all shapes in the model for any missing or invalid data." );
  }
}
/**
 * Update the 3D widget with a new object 
 */
void CreateSampleShapeDialog::update3DView()
{
  //Specific validation for the shapes
  if (validate())
  {
    std::string shapexml = constructShapeXML().toStdString();
    if( m_shapeTree->topLevelItemCount() > 0 && shapexml.empty() )
    {
      QMessageBox::information(this, "CreateSampleShapeDialog", 
			       "An error occurred while parsing the shape tree.\n"
			       "Please check that each node has two children and the lowest elements are primitive shapes.");
      return;
    
    }

    // Testing a predefined complex shape PLEASE LEAVE FOR THE MOMENT
  //   std::string shapexml = "<cuboid id=\"cuboid_1\" >\n"
  //     "<left-front-bottom-point x=\"-0.02\" y=\"-0.02\" z= \"0.0\" />\n"
  //     "<left-front-top-point x=\"-0.02\" y=\"0.05\" z= \"0.0\" />\n"
  //     "<left-back-bottom-point x=\"-0.02\" y=\"-0.02\" z= \"0.07\" />\n"
  //     "<right-front-bottom-point x=\"0.05\" y=\"-0.02\" z= \"0.0\" />\n"
  //     "</cuboid>\n"
  //     "<infinite-cylinder id=\"infcyl_1\" >"
  //     "<radius val=\"0.025\" />"
  //     "<centre x=\"0.015\" y=\"0.015\" z= \"0.07\" />"
  //     "<axis x=\"0.0\" y=\"0.0\" z= \"-0.001\" />"
  //     "</infinite-cylinder>\n"
  //     "<sphere id=\"sphere_1\">"
  //     "<centre x=\"0.015\" y=\"0.015\" z= \"0.035\" />"
  //     "<radius val=\"0.04\" />"
  //     "</sphere>\n"
  //     "<infinite-cylinder id=\"infcyl_3\" >"
  //     "<radius val=\"0.025\" />"
  //     "<centre x=\"0.015\" y=\"-0.02\" z= \"0.035\" />"
  //     "<axis x=\"0.0\" y=\"0.001\" z= \"0.0\" />"
  //     "</infinite-cylinder>\n"
  //     "<infinite-cylinder id=\"infcyl_2\" >"
  //     "<radius val=\"0.025\" />"
  //     "<centre x=\"-0.02\" y=\"0.015\" z= \"0.035\" />"
  //     "<axis x=\"0.001\" y=\"0.0\" z= \"0.0\" />"
  //     "</infinite-cylinder>\n"
  //     "<algebra val=\"((cuboid_1 sphere_1) (# (infcyl_1:(infcyl_2:infcyl_3))))\" />\n";


    Mantid::Geometry::ShapeFactory sFactory;
    boost::shared_ptr<Mantid::Geometry::Object> shape_sptr = sFactory.createShape(shapexml);
    //  std::cerr << "\n--------- XML String -----------\n" << shapexml << "\n---------------------\n";
    if( shape_sptr == boost::shared_ptr<Mantid::Geometry::Object>() ) return;
    try 
    {
      shape_sptr->initDraw();
    }
    catch( ... )
    {
      QMessageBox::information(this,"Create sample shape", 
			       QString("An error occurred while attempting to initialize the shape.\n") +
			       "Please check that all objects intersect each other.");
      return;
    }

    m_object_viewer->setDisplayObject(shape_sptr);
  }
  else
  {
    QMessageBox::critical(this, "", 
              "One or more shapes are invalid so the model cannot be rendered.\n"
              "Please check all shapes in the model for any missing or invalid data." );
  }
}

/**
 * This slot is called when a context menu is requested inside the tree widget
 * @param pos :: The position of the mouse pointer when the menu was requested
 */
void CreateSampleShapeDialog::handleTreeContextMenuRequest(const QPoint & pos)
{
  QMenu *context_menu = new QMenu(m_shapeTree);

  QTreeWidgetItem *item = m_shapeTree->itemAt(pos);
  QString op_text = "Insert child operation";
  bool is_shape(false);
  if( item )
  {
    QString displayText = item->text(0);
    if( !displayText.startsWith("inter") && !displayText.startsWith("uni") && 
	!displayText.startsWith("diff") )
    {  
      is_shape = true;
      //For a shape we need the option to mark it as a complement shape
      QAction *complement = new QAction("Complement", context_menu);
      complement->setCheckable(true);
      bool isChecked = m_details_map.value(getSelectedItem())->getComplementFlag();
      complement->setChecked(isChecked);
      connect(complement, SIGNAL(toggled(bool)), this, SLOT(toggleShapeComplement(bool)));
      context_menu->addAction(complement);
      context_menu->addSeparator();

      op_text = "Insert operation above";
    }

  }
  
  QMenu *add_op = new QMenu(op_text);
  add_op->addAction(new QAction("intersection", add_op));
  add_op->addAction(new QAction("union", add_op));
  add_op->addAction(new QAction("difference", add_op));
  connect(add_op, SIGNAL(triggered(QAction*)), this, SLOT(addOperation(QAction*)));
  context_menu->addMenu(add_op);

  if( !is_shape || m_shapeTree->topLevelItemCount() == 0 )
  {
    QMenu *submenu = new QMenu("Insert child shape");
    QStringList shapes = m_setup_map.keys();
    QStringListIterator itr(shapes);
    while( itr.hasNext() )
    {
      submenu->addAction(new QAction(itr.next(), submenu));
    }
    connect(submenu, SIGNAL(triggered(QAction*)), this, SLOT(addShape(QAction*)));
    context_menu->addMenu(submenu);
  }

  context_menu->addSeparator();
  QAction *remove = new QAction("Delete", context_menu);
  connect(remove, SIGNAL(triggered()), this, SLOT(handleDeleteRequest()));
  context_menu->addAction(remove);

  context_menu->popup(QCursor::pos());
}

void CreateSampleShapeDialog::toggleShapeComplement(bool state)
{
  BinaryTreeWidgetItem *selected = getSelectedItem();
  if( m_details_map.contains(selected) )
  {
    m_details_map.value(selected)->setComplementFlag(state);
  }
  if( state )
  {
    selected->setText(0, QString("# ") + selected->text(0));
  }
  else
  {
    selected->setText(0, selected->text(0).section('#', 1).trimmed());
  }

}

void CreateSampleShapeDialog::changeTreeData(BinaryTreeWidgetItem* item, int data)
{
  if( m_ops_map.contains(item) )
  {
    m_ops_map.value(item)->binaryop = data;
  }
}

BinaryTreeWidgetItem* CreateSampleShapeDialog::getSelectedItem()
{
  if( !m_shapeTree->selectedItems().isEmpty() )
  {
    /// Single selections are the only ones allowed
    return dynamic_cast<BinaryTreeWidgetItem*>(m_shapeTree->selectedItems()[0]);
  }
  
  // Check if tree is empty
  if( m_shapeTree->topLevelItemCount() == 0 )
  {
    // Give back the invisible root item
    return m_shapeTree->root();
  }
  else
  {
    QMessageBox::information(this, "CreateSampleShape", "Please select an item in the list as a parent.");
  }
  return NULL;
}

/**
 * Add a new child shape
 * @param shape :: The action that emitted the signal
 */
void CreateSampleShapeDialog::addShape(QAction *shape)
{
  // Get the selected item
  BinaryTreeWidgetItem *parent = getSelectedItem();
  if( parent && parent->childCount() == 2 ) return;

  BinaryTreeWidgetItem *child = new BinaryTreeWidgetItem(QStringList(shape->text()));
  child->setFlags(child->flags() & ~Qt::ItemIsEditable);

  if( m_shapeTree->topLevelItemCount() == 0 )
  {
    m_shapeTree->insertTopLevelItem(0, child);
  }
  else
  {
    parent->addChildItem(child);
  }

  // This calls setupDetails
  m_shapeTree->setCurrentItem(child);
  m_shapeTree->expandAll();
}

/** 
 * Add operation node based on menu action
 */
void CreateSampleShapeDialog::addOperation(QAction *opt)
{
  //Get the selected item
  BinaryTreeWidgetItem *selected = getSelectedItem();
  if( selected && selected->childCount() == 2 ) return;

  BinaryTreeWidgetItem *operation = new BinaryTreeWidgetItem;
  QFont font = operation->font(0);
  font.setBold(true);
  operation->setFont(0, font);
  operation->setData(0, Qt::DisplayRole, opt->text());
  int opcode(0);
  if( opt->text().startsWith("u") ) opcode = 1;
  else if( opt->text().startsWith("d") ) opcode = 2;
  else opcode = 0;

  operation->setData(0, Qt::UserRole, opcode);
  operation->setFlags(operation->flags() | Qt::ItemIsEditable);
  
  if( m_shapeTree->topLevelItemCount() == 0 )
  {
    m_shapeTree->insertTopLevelItem(0, operation);
  }
  else
  { 
    if( m_ops_map.contains(selected) )
    {
      selected->addChildItem(operation);
    }
    else if( selected->parent() )
    {
      int index  = selected->parent()->indexOfChild(selected);
      selected->parent()->insertChild(index, operation);
      selected->parent()->removeChild(selected);
      operation->addChildItem(selected);
    }
    else
    {
      m_shapeTree->takeTopLevelItem(m_shapeTree->indexOfTopLevelItem(selected));
      m_shapeTree->insertTopLevelItem(0, operation);
      operation->addChildItem(selected);
    }
  }

  m_ops_map.insert(operation, new Operation(opcode));
  // This calls setupDetails if necessary
  m_shapeTree->setCurrentItem(operation);
  m_shapeTree->expandAll();
  
}

/**
 * Handle a delete signal
 */
void CreateSampleShapeDialog::handleDeleteRequest()
{
  BinaryTreeWidgetItem *item = getSelectedItem();
  if( !item ) return;
  removeItem(item);
}

/**
 * Remove an item and all children from the tree
 */
void CreateSampleShapeDialog::removeItem(BinaryTreeWidgetItem *item)
{
  if( !item ) return;

  //Recursively remove children
  if( item->childCount() > 0 ) 
  {
    while( item->childCount() > 0 )
    {
      if( item->leftChild() ) removeItem(item->leftChild());
      if( item->rightChild() ) removeItem(item->rightChild());
    }
  }
  
  if( m_details_map.contains(item) )
  {
    ShapeDetails *obj = m_details_map.take(item);
    delete obj;
  }
  else if( m_ops_map.contains(item) )
  {
    Operation *obj = m_ops_map.take(item);
    delete obj;
  }
  else return;

  if( item->parent() )
  {
    item->parent()->removeChild(item);
  }
  else
  {
    m_shapeTree->takeTopLevelItem(m_shapeTree->indexOfTopLevelItem(item));
  }
}

/**
 * Setup the layout for the details box based upon the item given
 */
void CreateSampleShapeDialog::setupDetailsBox()
{
  QList<QTreeWidgetItem*> selection = m_shapeTree->selectedItems();
  if( selection.isEmpty() ) return;
  
  // Remove the current widget if one exists in the scroll area
  if( m_uiForm.details_scroll->widget() ) m_uiForm.details_scroll->takeWidget();

  BinaryTreeWidgetItem *item = dynamic_cast<BinaryTreeWidgetItem*>(selection[0]);
  QString shapename = item->text(0);
  if( m_setup_map.contains(shapename) )
  {
    ShapeDetails *obj = NULL; 
    if( m_details_map.contains(item) )
    {
      obj = m_details_map.value(item);
    }
    else 
    {
      obj = createDetailsWidget(shapename);
      m_details_map.insert(item, obj);
    }
    //Set it as the currently displayed widget
    m_uiForm.details_scroll->setWidget(obj);    
  }
  
}

/**
 * Create the correct type of details box for the shape
 * @param shapename :: The name of the shape for which to create a widget 
 * @return A pointer to the details object
 */
ShapeDetails* CreateSampleShapeDialog::createDetailsWidget(const QString & shapename) const
{
  if( m_setup_map.contains(shapename) )
  {
    return m_setup_map.value(shapename)->createInstance();
  }
  return NULL;
}

/**
 * Construct the XML from the current tree
 */
QString CreateSampleShapeDialog::constructShapeXML() const
{
  if( m_shapeTree->topLevelItemCount() == 0 || m_details_map.isEmpty() ) return QString();
  
  QString shapexml;
  // First construct the XML that builds each separately defined shape
  QMapIterator<BinaryTreeWidgetItem*, ShapeDetails*> detitr(m_details_map);
  while( detitr.hasNext() )
  {
    detitr.next();
    shapexml += detitr.value()->writeXML() + "\n";
  }
  
  QList<BinaryTreeWidgetItem*> postfix_exp;
  //Build expression list
  m_shapeTree->traverseInPostOrder(m_shapeTree->root(), postfix_exp);

  QListIterator<BinaryTreeWidgetItem*> expitr(postfix_exp);
  QStringList inter_results;
  while( expitr.hasNext() )
  {
    BinaryTreeWidgetItem* item = expitr.next();
    if( m_details_map.contains(item) )
    {
      ShapeDetails *shape = m_details_map.value(item);
      QString shapeID = shape->getShapeID();
      if( shape->getComplementFlag() )
      {
	      shapeID = QString("#(") + shapeID + QString(")");
      }
      inter_results.append(shapeID);
    }
    else if( m_ops_map.contains(item) )
    {
      int rcount = inter_results.count();
      if( inter_results.count() < 2 ) 
      {
	shapexml = "";
	break;
      }
      QString left = inter_results.at(rcount - 2);
      QString right = inter_results.at(rcount - 1);
      QString result = m_ops_map.value(item)->toString(left, right);
      // Remove left result and replace the right with the result
      inter_results.removeAt(rcount - 2);
      //List has now been reduced in size by 1
      inter_results.replace(rcount - 2, result);
    }
    else
    {
      shapexml = "";
      break;
    }
  }
  
  // Something went wrong if the list hasn't compacted down to one entry
  if( inter_results.size() != 1 || shapexml.isEmpty() ) 
  {
    return QString();
  }

  shapexml += "<algebra val=\"" + inter_results.at(0) + "\" />";
  return shapexml;
}


//=================================================================
//=================================================================

//------------------------------------------------
// BinaryTreeWidgetItem
//------------------------------------------------
/**
 * Default constructor
 * @param type :: The type of the item
 */
BinaryTreeWidgetItem::BinaryTreeWidgetItem(int type) 
  : QTreeWidgetItem(type), m_left_index(0), m_right_index(1)
{
}

/**
 * Construct an item with a string list of column texts to add
 * @param strings :: A list of strings to appear as the column texts
 * @param type :: Qt or User defined 
 */
BinaryTreeWidgetItem::BinaryTreeWidgetItem(const QStringList & strings, int type) 
  : QTreeWidgetItem(strings, type), m_left_index(0), m_right_index(1)
{
}

/**
 * Add a child item. This will only succeed if there are fewer than 2 children currently
 */
bool BinaryTreeWidgetItem::addChildItem(BinaryTreeWidgetItem* child)
{
  if( childCount() >= 2 ) return false;
  
  // Call sub-class function
  this->addChild(child);
  return true;
}

/**
 * A pointer to the left child. It can be NULL
 */
BinaryTreeWidgetItem* BinaryTreeWidgetItem::leftChild() const
{
  return dynamic_cast<BinaryTreeWidgetItem*>(this->child(m_left_index));
}

/**
 * A pointer to the right child. It can be NULL
 */
BinaryTreeWidgetItem* BinaryTreeWidgetItem::rightChild() const
{
  return dynamic_cast<BinaryTreeWidgetItem*>(this->child(m_right_index));
}

//------------------------------------------------
// BinaryTreeWidget
//------------------------------------------------

/**
 * Default constructor
 */
BinaryTreeWidget::BinaryTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
  ComboBoxDelegate *delegate = new ComboBoxDelegate(this);
  setItemDelegate(delegate);
}

/**
 * Gets the root item for the tree
 */
BinaryTreeWidgetItem* BinaryTreeWidget::root() const
{
  return dynamic_cast<BinaryTreeWidgetItem*>(invisibleRootItem()->child(0));
}

/**
 * A recursive function that builds an expression from the binary tree by traversing it in a post order fashion
 * @param node :: The parent node
 * @param expression :: The expression list to build
 */
void BinaryTreeWidget::traverseInPostOrder(BinaryTreeWidgetItem* node, QList<BinaryTreeWidgetItem*> & expression)
{
  if( !node ) return;
  //  For the time begin just print the string that we get
  if( node->leftChild() ) traverseInPostOrder(node->leftChild(), expression);
  if( node->rightChild() ) traverseInPostOrder(node->rightChild(), expression);

  // Append this to the list
  expression.append(node);
}

void BinaryTreeWidget::dataChanged(const QModelIndex & topLeft, const QModelIndex &)
{
  emit treeDataChange(dynamic_cast<BinaryTreeWidgetItem*>(itemFromIndex(topLeft)), 
		      topLeft.data(Qt::UserRole).toInt());
}

//------------------------------------------------
// ComboBoxDelegate
//------------------------------------------------
/**
 * Default constructor
 */
ComboBoxDelegate::ComboBoxDelegate(QWidget *parent) : QItemDelegate(parent) 
{
}

/**
 * Create an editor for a tree item
 * @param parent :: The parent widget
 * @param index :: unused argument
 * @param option :: unused argument
 */
QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
					  const QModelIndex &) const
{
  QComboBox *editor = new QComboBox(parent);
  editor->addItem("intersection");
  editor->addItem("union");
  editor->addItem("difference");
  
  return editor;
}

/**
 * Set the data for the editor when it has been created
 * @param editor :: The editor in question
 * @param index :: The model item in question
 */
void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  int value = index.model()->data(index, Qt::UserRole).toInt();
  
  QComboBox *combo_box = qobject_cast<QComboBox*>(editor);
  combo_box->setCurrentIndex(value);
}

/**
 * Set the data for the model when editing is finished
 * @param editor :: The editor in question
 * @param model :: The model in question
 * @param index :: The index for the model given
 */
void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
				   const QModelIndex &index) const
{
  QComboBox *combo_box = static_cast<QComboBox*>(editor);
  int boxitem = combo_box->currentIndex();
  QString value = combo_box->itemText(boxitem);

  model->setData(index, boxitem, Qt::UserRole);
  model->setData(index, value, Qt::DisplayRole);
}

/**
 * Set the appropriate geometry for the widget
 * @param editor :: The editor in question
 * @param option :: The style option
 * @param index :: The index for the model given
 */
void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, 
					   const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}
