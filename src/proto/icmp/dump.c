/**
 * @file icmp/dump.c
 * @brief dump function for icmp
 * @author simpart
 */
/*** include ***/
#include <stdio.h>
#include "pia/com.h"
#include "pia/ip.h"
#include "pia/icmp.h"

/*** function ***/
uint8_t piaicm_dump (piaicm_hdr_t * msg) {
    const char * str_buf = NULL;
    piaicm_echo_t * chk_echo = NULL;
    
    /* check parameter */
    if (NULL == msg) {
        PIA_ERROR("paramter is NULL");
        return PIA_NG;
    }
    
    /* dump type */
    str_buf = piaicm_gettype_str(msg);
    printf("icmp %s ", str_buf);
    
    /* check type */
    if (PIA_TRUE == piaicm_isecho(msg)) {
        /* this is echo message */
        chk_echo = (piaicm_echo_t *) piaicm_seekecho(msg);
        if (NULL == chk_echo) {
            PIA_ERROR("return is NULL");
            return PIA_NG;
        }
        printf("id=%u seq=%u", piaicm_getid(chk_echo), piaicm_getseq(chk_echo));
    } else {
        /* dumo code */
        str_buf = piaicm_getcode_str(msg);
        if (NULL == str_buf) {
            PIA_ERROR("return is NULL");
            return PIA_NG;
        }
        printf("code='%s' (0x%x)", str_buf, msg->code);
    }
    printf("\n");
    
    return PIA_OK;
}

uint8_t piaicm_dumpdtl (piaip_v4hdr_t * ip_hdr) {
    const char * str_buf = NULL;
    int msg_siz = 0;
    piaicm_hdr_t * msg = NULL;
    
    if (NULL == ip_hdr) {
        PIA_ERROR("paramter is NULL");
        return PIA_NG;
    }
    msg = (piaicm_hdr_t *) piaip_seekpld(ip_hdr);
    
    printf("ICMP message\n");
    printf("==========================\n");
    piaicm_dump_type(msg);
    
    if (PIA_TRUE == piaicm_isecho(msg)) {
        /* this is echo message */
        printf("code     : 0x%02x\n", msg->code);
        printf("checksum : 0x%04x\n", msg->chksum);
        piaicm_dump_id(msg);
        piaicm_dump_seq(msg);
        
        msg_siz = piaip_getpldsize(ip_hdr);
        if (PIA_NG == msg_siz) {
            return PIA_NG;
        }
        piaicm_dump_dat(
            msg,
            msg_siz - (sizeof(piaicm_hdr_t) + sizeof(piaicm_echo_t))
        );
    } else {
        /* dumo code */
        str_buf = piaicm_getcode_str(msg);
        if (NULL == str_buf) {
            PIA_ERROR("return is NULL");
            return PIA_NG;
        }
        printf("code     : '%s' (0x%x)", str_buf, msg->code);
        printf("checksum : 0x%04x\n", msg->chksum);
    }
    
    return PIA_OK;
}


const char * piaicm_gettype_str (piaicm_hdr_t * msg) {
    const char * tp_lst[] = {
        "echo reply"   ,             // 0x00
        "unknown"      ,             // 0x01
        "unknown"      ,             // 0x02
        "destination unreachable",   // 0x03
        "unknown"      ,             // 0x04
        "redirect"     ,             // 0x05
        "unknown"      ,             // 0x06
        "unknown"      ,             // 0x07
        "echo request" ,             // 0x08
        "unknown"      ,             // 0x09
        "unknown"      ,             // 0x0a
        "time exceeded"              // 0x0b
    };
    if (NULL == msg) {
        PIA_ERROR("paramter is NULL");
        return NULL;
    }
    if (PIAICM_TYPE_TMEXCD  >= msg->type) {
        return tp_lst[msg->type];
    }
    return NULL;
}

const char * piaicm_getcode_str (piaicm_hdr_t * msg) {
    uint8_t code     = 0;
    const char *  unrch_lst[] = {
        "net unreachable"                ,
        "host unreachable"               ,
        "protocol unreachable"           ,
        "port unreachable"               ,
        "fragment needed and df was set" ,
        "source route failed"            ,
        "destination network unknown"    ,
        "destination host unknown"       ,
        "source host isolated"           ,
        "communication with destination network is administratively prohibited" ,
        "communication with destinaltion host is administratively prohibited"   ,
        "destination network unreachable for tos"  ,
        "destination host unreachable for tos"     ,
        "communication administratively prohibited",
        "host precedence violation"                ,
        "precedence cutoff in effect"
    };
    const char * rdct_lst[] = {
        "redirect datagram for the network"         ,
        "redirect datagram for the host"            ,
        "redirect datagram for the tos and network" ,
        "redirect datagram for the tos and host"
    };
    const char * tmex_lst[] = {
        "time to live exceeded in transit",
        "fragment reassembly time exceeded"
    };
    
    if (NULL == msg) {
        return NULL;
    }
    
    if (PIA_TRUE == piaicm_isecho(msg)) {
        return NULL;
    }
    
    code = msg->code;
    switch (code) {
        case PIAICM_TYPE_DSTUNRCH:
            if (PIAICM_DUNR_PRCDCF >= code) {
                return unrch_lst[code];
            }
            break;
        case PIAICM_TYPE_REDIRECT:
            if (PIAICM_RDCT_DGTH >= code) {
                return rdct_lst[code];
            }
            break;
        case PIAICM_TYPE_TMEXCD:
            if ( (PIAICM_TMEX_TTL == code) || (PIAICM_TMEX_FGR == code) ) {
                return tmex_lst[code];
            }
            break;
    }
    return NULL;
}

uint8_t piaicm_dump_type (piaicm_hdr_t * msg) {
    const char * type_str = piaicm_gettype_str(msg);
    if (NULL == type_str) {
        return PIA_NG;
    }
    printf("type     : %s\n", type_str);
    
    return PIA_OK;
}

uint8_t piaicm_dump_id(piaicm_hdr_t * msg) {
    uint8_t * seek = NULL;
    
    seek = piaicm_seekecho(msg);
    if (NULL == seek) {
        PIA_ERROR("return is NULL");
        return PIA_NG;
    }
    printf("id       : %u\n", piaicm_getid((piaicm_echo_t *)seek));
    return PIA_OK;
}

uint8_t piaicm_dump_seq(piaicm_hdr_t * msg) {
    uint8_t * seek = NULL;

    seek = piaicm_seekecho(msg);
    if (NULL == seek) {
        PIA_ERROR("return is NULL");
        return PIA_NG;
    }
    printf("sequence : %u\n", piaicm_getseq((piaicm_echo_t *)seek));
    return PIA_OK;
}

uint8_t piaicm_dump_dat(piaicm_hdr_t * msg, size_t siz) {
    uint8_t * seek = NULL;
    int       loop = 0;
    
    seek = piaicm_seekecho(msg);
    if (NULL == seek) {
        PIA_ERROR("return is NULL");
        return PIA_NG;
    }
    seek = piaicm_seekecho_dat((piaicm_echo_t *) seek);
    if (NULL == seek) {
        PIA_ERROR("return is NULL");
        return PIA_NG;
    }
    printf("data     : (%u byte)\n", (int)siz);
    for (loop=1; loop <= (int)siz ;loop++) {
        if (0 == ((loop-1)%8)) {
            printf("           ");
        }
        printf("%02x", *(seek+(loop-1)));
        if (1 != loop) {
            if ((0 == (loop%2)) && (0 != (loop%8))) {
                printf(" ");
            } else if (0 == (loop%8)) {
                printf("\n");
            }
        }
    }
    return PIA_OK;
}
/* end of file */
