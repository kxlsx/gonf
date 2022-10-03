#ifndef _GONF_DUMP_H
/* ================= */
#define _GONF_DUMP_H 1

/* head & tail of the gonf lib file */
#define gonf_head_c_dump res_gonf_head_c
#define gonf_head_c_dump_len res_gonf_head_c_len
#define gonf_tail_c_dump res_gonf_tail_c
#define gonf_tail_c_dump_len res_gonf_tail_c_len
extern unsigned char res_gonf_head_c[];
extern unsigned int res_gonf_head_c_len;
extern unsigned char res_gonf_tail_c[];
extern unsigned int res_gonf_tail_c_len;

/* head & tail of the gonf header file */
#define gonf_head_h_dump res_gonf_head_h
#define gonf_head_h_dump_len res_gonf_head_h_len
#define gonf_tail_h_dump res_gonf_tail_h
#define gonf_tail_h_dump_len res_gonf_tail_h_len
extern unsigned char res_gonf_head_h[];
extern unsigned int res_gonf_head_h_len;
extern unsigned char res_gonf_tail_h[];
extern unsigned int res_gonf_tail_h_len;

/* ================= */
#endif