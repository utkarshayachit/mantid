/******************************************************************/
/*
  This file is a SIP file for binding C++ methods of the VatesSimpleGui to Python.
*/
/******************************************************************/

// Define the module name. This has to match the library filename
%Module mantidvsipython

/******************************** SIP Imports *********************/
%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

/******************************************************************/
/*
namespace MantidQt
{
namespace API
{

class VatesViewerInterface : public QWidget {
%TypeHeaderCode
#include "MantidQtAPI/VatesViewerInterface.h"
%End

public:
    VatesViewerInterface();
};

}; // namespace API
}; // namespace MantidQt

namespace Mantid
{
namespace Vates
{
namespace SimpleGui
{
namespace MantidQt.API;

class MdViewerWidget : public MantidQt.API.VatesViewerInterface {
%TypeHeaderCode
#include "MantidVatesSimpleGuiViewWidgets/MdViewerWidget.h"
%End

public:
    MdViewerWidget();
    void setupPluginMode();
};

}; // namespace SimpleGui
}; // namespace Vates
}; // namespace Mantid
*/


namespace Mantid
{
namespace Vates
{
namespace SimpleGui
{

class MdViewerWidget : public QWidget {
%TypeHeaderCode
#include "MantidVatesSimpleGuiViewWidgets/MdViewerWidget.h"
%End

public:
    MdViewerWidget();
    void setupPluginMode();
};

}; // namespace SimpleGui
}; // namespace Vates
}; // namespace Mantid

