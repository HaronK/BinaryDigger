/*
 * TsuTemplate.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: Oleg Khryptul
 */

#include <default_plugin.h>

#define ACDF_MAGIC_STR_LENGTH 8 /** length of magic part in header. Do not change!  */

typedef long long AC_DATETIME; /**< actual datatype for AC_DT_DATETIME */
typedef int AC_INT; /**< actual datatype for AC_DT_INTEGER, AC_DT_DATE and AC_DT_TIME  */
typedef double AC_DOUBLE; /**< actual datatype for AC_DT_FLOAT */
typedef unsigned int ACCRC32;

TEMPL(AC_DATAFILE_HEADER_SECURE)
    VAR(DWORD, trans_no);               /**< number of latest transaction written to datafile */
//#ifdef AC_DATAFILE64
    VAR(DWORD, offsets_multiplier);                    /**< offset multiplier */
//#else
//  AC_INT reserved1;                             /**< reserved 1 */
//#endif
    VAR(QWORD, last_datetime_trans);              /**< last transaction time */
    VAR(QWORD, datetime_last_suspect_record);     /**< timestamp of last susp. rec. */
    VAR(DWORD, num_suspect_records);                   /**< number of suspect records */
    VAR(DWORD, num_suspect_record_items);              /**< number of suspect record items */
    VAR(DWORD, num_drecords);                  /**< nr. of data records */
    VAR(DWORD, num_dblocks);                           /**< nr. of blocks */
    VAR(DWORD, num_crecords);                  /**< nr. of correction records */
    VAR(DWORD, formula_length);                /**< length of formula */
    VAR(DWORD, cache_length);                          /**< length of cache */
    VAR(DWORD, text_length);                           /**< length of the text */
    VAR(DWORD, update_user);                           /**< ID of user that executed last transaction */
    VAR(DWORD, reserved3);                             /**< reserved 3 */
    VAR(DWORD, header_data_offset);                    /**< file offset header info: formula*/
    VAR(DWORD, index_offset);                          /**< file offset block index */
    VAR(DWORD, crc32);                                /**< 32 bit crc check over header */
    VAR(DWORD, trans_no2);                              /**< copy of trans_no to check whether this header part was written completely */
TEMPL_END

TEMPL(AC_DATAFILE_HEADER)
    ARR(CHAR, magic, ACDF_MAGIC_STR_LENGTH);
    VAR(QWORD, datetime_first_trans);
    /* security */
    VAR(DWORD, permissions);                       /**< permission bits */
    VAR(DWORD, filegroup);                     /**< group file is in */
    /* source info */
    VAR(DWORD, time_zone);                             /**< time zone number, -1 is server, -2 = GMT, not used yet */

    VAR(DWORD, num_fields);                /**< nr. of fields */
    VAR(DWORD, num_extra_key_fields);                  /**< nr. of non-DATE, TIME fields constituting the key */
    VAR(DWORD, contents_mask);                         /**< whether intra-day/second or not */

    VAR(DWORD, crecord_size);
    VAR(DWORD, drecord_size_fixed_part);
    /* info for derived time series */
    VAR(DWORD, recalc_trigger);            /**< recalculation trigger */
    VAR(DWORD, max_maturity);                          /**< refresh if older than .. secs */
    VAR(QWORD, datetime_last_compute);            /**< latest refresh */
    /* automatic clean up info */
    VAR(DWORD, max_num_records);                       /**< maximum of data records */
    VAR(DWORD, max_maturity_records);                  /**< maximum maturity of records in days */
    /* validation info */
    VAR(DWORD, cal_num);               /**< cal. nr. that is applicable */
    VAR(DWORD, check_func_no);                 /**< validation functions check mask */
    VAR(DOUBLE, check_percentage);               /**< e.g. percentage change */
    VAR(DOUBLE, check_factor_stdev);                 /**< not in use */
    VAR(DWORD, check_number_of_records);               /**< not in use */
    VAR(DWORD, check_dummy);                           /**< not in use */

    VAR(DWORD, max_logfile_size);                      /**< when to split */
    VAR(DWORD, other_int_dummy);                       /**< dummy int to get 4 ints after last double */

    VAR(QWORD, datetime_trigger_compute);         /**< trigger computation time */
    ARR(CHAR, unused, 24);              /**< room for future expansion: in total 144 bytes */
    VAR(AC_DATAFILE_HEADER_SECURE, sec1);               /**< header part with references to header data and index, sec2 may be more recent */
    VAR(AC_DATAFILE_HEADER_SECURE, sec2);               /**< header part with references to header data and index, sec1 may be more recent */
TEMPL_END

TEMPL(SecureHeaderExtra, const AC_DATAFILE_HEADER_SECURE& sec)
TEMPL_END

TEMPL(Tsu)
    VAR(AC_DATAFILE_HEADER, header);
    GET(header, DWORD, num_fields);
    ARR(WORD, field_list, num_fields);
    ARR(WORD, field_types, num_fields);

    GET(header, DWORD, num_extra_key_fields);

    if (num_extra_key_fields > 0)
    {
        ARR(WORD, extra_key_list, num_extra_key_fields);
    }

//    GET(header, AC_DATAFILE_HEADER_SECURE, sec1);
//    GET(header, AC_DATAFILE_HEADER_SECURE, sec2);

//    if (sec1.get<DWORD>("trans_no") >= sec2.get<DWORD>("trans_no"))
//    {
//        VAR_TP(SecureHeaderExtra, secExtra1)(sec1);
//        VAR_TP(SecureHeaderExtra, secExtra2)(sec2);
//    }
//    else
//    {
//        VAR_TP(SecureHeaderExtra, secExtra1)(sec2);
//        VAR_TP(SecureHeaderExtra, secExtra2)(sec1);
//    }
//    VAR_TP(NpkEntry, npkEntries)(npkHeader->get<DWORD>("dataOffset") + 8); // <open=true>;
//    VAR_T(NpkDataHeader, npkDataHeader);
TEMPL_END

PLUGIN(TimeSeriesUpdate)
    TEMPL_REGISTER(Tsu);
PLUGIN_END
