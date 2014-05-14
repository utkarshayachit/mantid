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

// Push import of mantidqt.sip to end to avoid Vates namespace being 
// placed in that module.

%Import mantidqt.sip
