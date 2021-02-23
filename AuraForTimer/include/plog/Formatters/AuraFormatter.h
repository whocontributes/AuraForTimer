#pragma once
#include <plog/Record.h>
#include <plog/Util.h>
#include <iomanip>
#include <stdlib.h>

namespace plog
{
    template<bool useUtcTime>
    class AuraFormatterImpl
    {
    public:
        static util::nstring header()
        {
            return util::nstring();
        }

        static util::nstring format(const Record& record)
        {
            tm t;
            useUtcTime ? util::gmtime_s(&t, &record.getTime().time) : util::localtime_s(&t, &record.getTime().time);


			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s(record.getFile(), NULL, 0, NULL, 0, fname,
				_MAX_FNAME, ext, _MAX_EXT);

            util::nostringstream ss;
            ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
            ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
            ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
			ss << PLOG_NSTR(" ") << fname << ext << PLOG_NSTR("::") << record.getFunc() << PLOG_NSTR(":") << record.getLine() << PLOG_NSTR(" ");

            ss << PLOG_NSTR("[Thread-") << record.getTid() << PLOG_NSTR("] ");
            ss << record.getMessage() << PLOG_NSTR("\n");

            return ss.str();
        }
    };

    class AuraFormatter : public AuraFormatterImpl<false> {};
    class AuraFormatterUtcTime : public AuraFormatterImpl<true> {};
}
