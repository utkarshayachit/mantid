#ifndef MANTIDQTCUSTOMINTERFACES_CONVERTTOENERGY_H_
#define MANTIDQTCUSTOMINTERFACES_CONVERTTOENERGY_H_

//----------------------
// Includes
//----------------------
#include "MantidQtCustomInterfaces/ui_ConvertToEnergy.h"
#include "MantidQtAPI/UserSubWindow.h"


namespace MantidQt
{
  namespace CustomInterfaces
  {
    //-------------------------------------------
    // Forward declarations
    //-------------------------------------------
    class Homer;
    class Indirect;

    /** 
    This class defines the ConvertToEnergy interface. It handles the overall instrument settings
    and sets up the appropriate interface depending on the deltaE mode of the instrument. The deltaE
    mode is defined in the instrument definition file using the "deltaE-mode".    

    @author Martyn Gigg, Tessella Support Services plc
    @author Michael Whitty

    Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>    
    */

    class ConvertToEnergy : public MantidQt::API::UserSubWindow
    {
      Q_OBJECT

    public: // public constants, ennumerations and types
      enum DeltaEMode { Direct, InDirect, Undefined};

    public: // public constructor, destructor and functions
      /// Default Constructor
      ConvertToEnergy(QWidget *parent = 0);
      ///Destructor
      ~ConvertToEnergy();
      /// Interface name
      static std::string name() { return "ConvertToEnergy"; }
      /// Aliases for this interface
      static std::set<std::string> aliases()
      { 
        std::set<std::string> aliasList;
        aliasList.insert("Homer");
        return aliasList;
      }

    private slots:
      void helpClicked();
      void runClicked();

    private: // private functions (and slots)
      /// Initialize the layout
      virtual void initLayout();
      virtual void initLocalPython();
      /// setup Instrument Selection QComboBox 's
      void setupCbInst();
      void readSettings();
      void saveSettings();
      void setDefaultInstrument(const QString & name = "");
      void instrumentSelectChanged(const QString& prefix);
      /// Find path to instrument's _Definition.xml file (and check there is a parameter file).
      QString getIDFPath(const QString& prefix);
      /// Find the DeltaEMode (Direct or Indirect) of the instrument.
      DeltaEMode instrumentDeltaEMode(const QString& defFile);
      /// Change the user interface to show the relevant sections for the instrument type.
      void changeInterface(DeltaEMode desired);
    private slots:
      void userSelectInstrument(const QString& prefix);

    private: // member variables
      /// The .ui form generated by Qt Designer
      Ui::ConvertToEnergy m_uiForm;
      /// Direct Instruments interface object
      Homer *m_directInstruments;
      /// Indirect Instruments interface object
      Indirect *m_indirectInstruments;
      /// Instrument the interface is currently set for.
      QString m_curInterfaceSetup;
      /// "DeltaE-mode" of the current instrument.
      DeltaEMode m_curEmodeType;
      /// The settings group
      QString m_settingsGroup;

    };

  }
}

#endif //MANTIDQTCUSTOMINTERFACES_CONVERTTOENERGY_H_
