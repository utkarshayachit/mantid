#ifndef MANTID_ICAT_CATALOGHELPER_H_
#define MANTID_ICAT_CATALOGHELPER_H_

#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"

namespace Mantid
{
  namespace ICat
  {
    class CatalogHelper
    {
      public:


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
            // Add rows headers to the output workspace.
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

      private:
        // Convert a file size to human readable file format.
        std::string bytesToString(int64_t &fileSize);
        // Helper method that formats a given timestamp.
        std::string formatDateTime(const time_t &timestamp, const std::string &format);
    };
  }
}

#endif //MANTID_ICAT_CATALOGHELPER_H_
