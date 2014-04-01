#include "MantidICat/CatalogHelper.h"
#include "MantidKernel/DateAndTime.h"

namespace Mantid
{
  namespace ICat
  {
    /**
     * Convert a file size to human readable file format.
     * @param fileSize :: The size in bytes of the file.
     */
    std::string CatalogHelper::bytesToString(int64_t &fileSize)
    {
      const char* args[] = {"B", "KB", "MB", "GB"};
      std::vector<std::string> units(args, args + 4);

      unsigned order = 0;

      while (fileSize >= 1024 && order + 1 < units.size())
      {
        order++;
        fileSize = fileSize / 1024;
      }

      return boost::lexical_cast<std::string>(fileSize) + units.at(order);
    }

    /**
     * Formats a given timestamp to human readable datetime.
     * @param timestamp :: Unix timestamp.
     * @param format    :: The desired format to output.
     * @return string   :: Formatted Unix timestamp.
     */
    std::string CatalogHelper::formatDateTime(const time_t &timestamp, const std::string &format)
    {
      return Kernel::DateAndTime(boost::posix_time::from_time_t(timestamp)).toFormattedString(format);
    }
  }
}
