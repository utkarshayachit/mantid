#ifndef MANTID_ICAT_CATALOGHELPER_H_
#define MANTID_ICAT_CATALOGHELPER_H_

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
    };
  }
}

#endif //MANTID_ICAT_CATALOGHELPER_H_
