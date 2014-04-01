#ifndef MANTID_ICAT_CATALOGHELPER_H_
#define MANTID_ICAT_CATALOGHELPER_H_

#include <iostream>
#include <stdexcept>

namespace Mantid
{
  namespace ICat
  {
    class CatalogHelper
    {
      public:

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
