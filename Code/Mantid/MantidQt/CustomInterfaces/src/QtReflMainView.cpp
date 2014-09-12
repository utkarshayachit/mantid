#include "MantidQtCustomInterfaces/QtReflMainView.h"
#include "MantidQtCustomInterfaces/QReflTableModel.h"
#include "MantidQtCustomInterfaces/ReflNullMainViewPresenter.h"
#include "MantidQtCustomInterfaces/ReflMainViewPresenter.h"
#include "MantidQtCustomInterfaces/ReflBlankMainViewPresenter.h"
#include "MantidQtCustomInterfaces/ReflLoadedMainViewPresenter.h"
#include "MantidAPI/ITableWorkspace.h"
#include <qinputdialog.h>
#include <qmessagebox.h>

namespace MantidQt
{
  namespace CustomInterfaces
  {
    using namespace Mantid::API;

    DECLARE_SUBWINDOW(QtReflMainView);

    //----------------------------------------------------------------------------------------------
    /** Constructor
    */
    QtReflMainView::QtReflMainView(QWidget *parent) : UserSubWindow(parent), m_presenter(new ReflNullMainViewPresenter())
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
    */
    QtReflMainView::~QtReflMainView()
    {
    }

    /**
    Initialise the Interface
    */
    void QtReflMainView::initLayout()
    {
      ui.setupUi(this);
      ui.workspaceSelector->refresh();

      //Expand the process runs column at the expense of the search column
      ui.splitterTables->setStretchFactor(0, 0);
      ui.splitterTables->setStretchFactor(1, 1);

      connect(ui.workspaceSelector,SIGNAL(activated(QString)),this,SLOT(setModel(QString)));
      connect(ui.buttonSave, SIGNAL(clicked()),this, SLOT(saveButton()));
      connect(ui.buttonSaveAs, SIGNAL(clicked()),this, SLOT(saveAsButton()));
      connect(ui.buttonNew, SIGNAL(clicked()),this, SLOT(setNew()));
      connect(ui.buttonAddRow, SIGNAL(clicked()),this, SLOT(addRowButton()));
      connect(ui.buttonDeleteRow, SIGNAL(clicked()),this, SLOT(deleteRowButton()));
      connect(ui.buttonProcess, SIGNAL(clicked()),this, SLOT(processButton()));
      setNew();
    }

    /**
    This slot loads a blank table and changes to a BlankMainView presenter
    */
    void QtReflMainView::setNew()
    {
      boost::scoped_ptr<IReflPresenter> newPtr(new ReflBlankMainViewPresenter(this));
      m_presenter.swap(newPtr);
    }

    /**
    This slot loads a table workspace model and changes to a LoadedMainView presenter
    @param name : the string name of the workspace to be grabbed
    */
    void QtReflMainView::setModel(QString name)
    {
      boost::scoped_ptr<IReflPresenter> newPtr(new ReflLoadedMainViewPresenter(name.toStdString(), this));
      m_presenter.swap(newPtr);
      m_presenter->notify(NoFlags);
    }

    /**
    Set a new model in the tableview
    @param model : the model to be attached to the tableview
    */
    void QtReflMainView::showTable(ITableWorkspace_sptr model)
    {
      ui.viewTable->setModel(new QReflTableModel(model));
      ui.viewTable->resizeColumnsToContents();
    }

    /**
    This slot notifies the presenter that the "save" button has been pressed
    */
    void QtReflMainView::saveButton()
    {
      m_presenter->notify(SaveFlag);
    }

    /**
    This slot notifies the presenter that the "save as" button has been pressed
    */
    void QtReflMainView::saveAsButton()
    {
      m_presenter->notify(SaveAsFlag);
    }

    /**
    This slot notifies the presenter that the "add row" button has been pressed
    */
    void QtReflMainView::addRowButton()
    {
      m_presenter->notify(AddRowFlag);
    }

    /**
    This slot notifies the presenter that the "delete" button has been pressed
    */
    void QtReflMainView::deleteRowButton()
    {
      m_presenter->notify(DeleteRowFlag);
    }

    /**
    This slot notifies the presenter that the "process" button has been pressed
    */
    void QtReflMainView::processButton()
    {
      m_presenter->notify(ProcessFlag);
    }

    /**
    Show an information dialog
    @param prompt : The prompt to appear on the dialog
    @param title : The text for the title bar of the dialog
    */
    void QtReflMainView::giveUserInfo(std::string prompt, std::string title)
    {
      QMessageBox::information(this,QString(title.c_str()),QString(prompt.c_str()),QMessageBox::Ok, QMessageBox::Ok);
    }

    /**
    Show an critical error dialog
    @param prompt : The prompt to appear on the dialog
    @param title : The text for the title bar of the dialog
    */
    void QtReflMainView::giveUserCritical(std::string prompt, std::string title)
    {
      QMessageBox::critical(this,QString(title.c_str()),QString(prompt.c_str()),QMessageBox::Ok, QMessageBox::Ok);
    }

    /**
    Show a warning dialog
    @param prompt : The prompt to appear on the dialog
    @param title : The text for the title bar of the dialog
    */
    void QtReflMainView::giveUserWarning(std::string prompt, std::string title)
    {
      QMessageBox::warning(this,QString(title.c_str()),QString(prompt.c_str()),QMessageBox::Ok, QMessageBox::Ok);
    }

    /**
    Ask the user a Yes/No question
    @param prompt : The prompt to appear on the dialog
    @param title : The text for the title bar of the dialog
    @returns a boolean true if Yes, false if No
    */
    bool QtReflMainView::askUserYesNo(std::string prompt, std::string title)
    {
      auto response = QMessageBox::question(this,QString(title.c_str()),QString(prompt.c_str()),QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
      if (response == QMessageBox::Yes)
      {
        return true;
      }
      return false;
    }

    /**
    Ask the user to enter a string.
    @param prompt : The prompt to appear on the dialog
    @param title : The text for the title bar of the dialog
    @param defaultValue : The default value entered.
    @returns The user's string if submitted, or an empty string
    */
    std::string QtReflMainView::askUserString(const std::string& prompt, const std::string& title, const std::string& defaultValue)
    {
      bool ok;
      QString text = QInputDialog::getText(QString::fromStdString(title), QString::fromStdString(prompt), QLineEdit::Normal, QString::fromStdString(defaultValue), &ok);
      if(ok)
        return text.toStdString();
      return "";
    }

    /**
    Get the indices of the highlighted rows
    @returns a vector of unsigned ints contianing the highlighted row numbers
    */
    std::vector<size_t> QtReflMainView::getSelectedRowIndexes() const
    {
      auto selectedRows = ui.viewTable->selectionModel()->selectedRows();
      //auto selectedType = ui.viewTable->selectionModel()->;
      std::vector<size_t> rowIndexes;
      for (auto idx = selectedRows.begin(); idx != selectedRows.end(); ++idx)
      {
        rowIndexes.push_back(idx->row());
      }
      return rowIndexes;
    }

  } // namespace CustomInterfaces
} // namespace Mantid