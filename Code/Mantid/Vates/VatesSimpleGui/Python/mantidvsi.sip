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

%Import mantidqt.sip

/******************************************************************/

namespace Mantid
{
namespace Vates
{
namespace SimpleGui
{

class MdViewerWidget : public MantidQt::API::VatesViewerInterface {
%TypeHeaderCode
#include "MantidVatesSimpleGuiViewWidgets/MdViewerWidget.h"
%End

public:
    MdViewerWidget();
    //void setupPluginMode();
};

}; // namespace SimpleGui
}; // namespace Vates
}; // namespace Mantid

