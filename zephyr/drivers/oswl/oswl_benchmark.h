#ifndef __OSWL_BENCHMARK_H__
#define __OSWL_BENCHMARK_H__
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum{
    OSWL_BENCHMARK_SYS_ID_SOFTWARE=1,
    OSWL_BENCHMARK_SYS_ID_HARDWARE,
    OSWL_BENCHMARK_SYS_ID_BUTT=63
}OSWL_BENCHMARK_SYS_ID;

typedef enum{
    OSWL_BENCHMARK_MAPI_MODULE_ID_SENSOR=1,
    OSWL_BENCHMARK_MAPI_MODULE_ID_VI,
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP,  //3
    OSWL_BENCHMARK_MAPI_MODULE_ID_FILTER,
    OSWL_BENCHMARK_MAPI_MODULE_ID_VPSS,
    OSWL_BENCHMARK_MAPI_MODULE_ID_REGION,
    OSWL_BENCHMARK_MAPI_MODULE_ID_VENC,
    OSWL_BENCHMARK_MAPI_MODULE_ID_JPEG,
    OSWL_BENCHMARK_MAPI_MODULE_ID_SVP,
    OSWL_BENCHMARK_MAPI_MODULE_ID_AI,
    OSWL_BENCHMARK_MAPI_MODULE_ID_AO,
    OSWL_BENCHMARK_MAPI_MODULE_ID_AENC,
    OSWL_BENCHMARK_MAPI_MODULE_ID_ADEC,
    OSWL_BENCHMARK_MAPI_MODULE_ID_SYS,
    OSWL_BENCHMARK_MAPI_MODULE_ID_AUDIO,    
    OSWL_BENCHMARK_MAPI_MODULE_ID_ALG,
    OSWL_BENCHMARK_MAPI_MODULE_ID_MIPI,
    OSWL_BENCHMARK_MAPI_MODULE_ID_SECURITY,
    OSWL_BENCHMARK_MAPI_MODULE_ID_SERVICE_API,
    OSWL_BENCHMARK_MAPI_MODULE_ID_VO,    
    OSWL_BENCHMARK_MAPI_MODULE_ID_DISPLAY,
    OSWL_BENCHMARK_MAPI_MODULE_ID_IMU,
    OSWL_BENCHMARK_MAPI_MODULE_ID_JSON,
    OSWL_BENCHMARK_MAPI_MODULE_ID_LENS,
    OSWL_BENCHMARK_MAPI_MODULE_ID_RTSP,
    OSWL_BENCHMARK_MAPI_MODULE_ID_STREAMER,
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_FE,   //27
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_BE,   //28
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_PE,   //29
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_GDC,  //30   
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_H3A,  //31 
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_LLI,  //32
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_CTL_TOP, //33           
    OSWL_BENCHMARK_MAPI_MODULE_ID_ISP_EIS, //34
    OSWL_BENCHMARK_MAPI_MODULE_ID_BUTT=63
}OSWL_BENCHMARK_MAPI_MODULE_ID;

typedef enum{
    OSWL_BENCHMARK_LAPI_MODULE_ID_SENSOR=1,
    OSWL_BENCHMARK_LAPI_MODULE_ID_VI,
    OSWL_BENCHMARK_LAPI_MODULE_ID_ISP,
    OSWL_BENCHMARK_LAPI_MODULE_ID_FILTER,
    OSWL_BENCHMARK_LAPI_MODULE_ID_VPSS,
    OSWL_BENCHMARK_LAPI_MODULE_ID_REGION,
    OSWL_BENCHMARK_LAPI_MODULE_ID_VENC,
    OSWL_BENCHMARK_LAPI_MODULE_ID_JPEG,    
    OSWL_BENCHMARK_LAPI_MODULE_ID_SVP,
    OSWL_BENCHMARK_LAPI_MODULE_ID_AI,
    OSWL_BENCHMARK_LAPI_MODULE_ID_AO,
    OSWL_BENCHMARK_LAPI_MODULE_ID_AENC,
    OSWL_BENCHMARK_LAPI_MODULE_ID_ADEC,
    OSWL_BENCHMARK_LAPI_MODULE_ID_SYS,    
    OSWL_BENCHMARK_LAPI_MODULE_ID_BUTT=63
}OSWL_BENCHMARK_LAPI_MODULE_ID;


typedef enum {
    OSWL_BENCHMARK_RECORD_ATTR_KEEP=0,
    OSWL_BENCHMARK_RECORD_ATTR_CLEAR,
    OSWL_BENCHMARK_RECORD_ATTR_REMOVE,
    OSWL_BENCHMARK_RECORD_ATTR_REMOVE_ALL,
}OSWL_BENCHMARK_RM_ATTR;

#define OSWL_BENCH_MARK_STRUCT_BASE()\
    unsigned long long total_time;\
    unsigned long long count_times;\
    unsigned long long last_record_time;\
    unsigned long long total_time_user;\
    unsigned long long last_record_time_user;\
    unsigned long long total_time_sys;\
    unsigned long long last_record_time_sys;\
    unsigned long long record_start_time;\
    unsigned int max_record_time;\
    unsigned int min_record_time;\
    unsigned int line_number_start;\
    unsigned int line_number_end;\
    unsigned int module_id;\
    unsigned int number_key;\
    char function_name[];\

typedef struct oswl_benchmark_out{
    OSWL_BENCH_MARK_STRUCT_BASE()
}oswl_benchmark_out_t;

typedef void (*dump_record)(oswl_benchmark_out_t* record,void* user_data);

extern unsigned long long g_oswl_benchmark_enable_flag_mapi;
extern unsigned long long g_oswl_benchmark_enable_flag_lapi;
extern unsigned long long g_oswl_benchmark_enable_flag_subid;

#define RECORD_MASK(ID,FLAG)\
    (FLAG&(1<<ID))

#define RECORD_ENABLE(MAPI_ID,LAPI_ID,SUB_ID)\
       (RECORD_MASK(MAPI_ID,g_oswl_benchmark_enable_flag_mapi)!=0 && \
       RECORD_MASK(LAPI_ID,g_oswl_benchmark_enable_flag_lapi)!=0 && \
       RECORD_MASK(SUB_ID,g_oswl_benchmark_enable_flag_subid)!=0)


#define OSWL_BENCHMARK_MODULE_ID(SYS_ID,MAPI_ID,LAPI_ID,SUB_ID)\
    (SYS_ID<<24|MAPI_ID<<16|LAPI_ID<<8|SUB_ID)
#define OSWL_TURNOFF_BENCHMARK
#ifdef OSWL_TURNOFF_BENCHMARK
#define OSWL_BENCHMARK_START_RECORD(MODULE_ID)
#define OSWL_BENCHMARK_END_RECORD(MODULE_ID)
#else
#define OSWL_BENCHMARK_START_RECORD(MODULE_ID)\
{\
    int tmp_oswl_benchmark_number_key=0;\
do{\
    if(RECORD_ENABLE(((MODULE_ID>>16)&0xff),((MODULE_ID>>8)&0xff),(MODULE_ID&0xff))){\
        tmp_oswl_benchmark_number_key=__LINE__;\
        oswl_benchmark_start(MODULE_ID,__LINE__,(char*)__FUNCTION__,tmp_oswl_benchmark_number_key);\
    }\
}while(0)

#define OSWL_BENCHMARK_END_RECORD(MODULE_ID)\
do{\
    if(RECORD_ENABLE(((MODULE_ID>>16)&0xff),((MODULE_ID>>8)&0xff),(MODULE_ID&0xff))){\
        oswl_benchmark_end(MODULE_ID,__LINE__,(char*)__FUNCTION__,tmp_oswl_benchmark_number_key);\
    }\
}while(0);\
}

#endif

int oswl_benchmark_start(unsigned int module_id,
                                   unsigned int line_number,
                                   char* function_name,
                                   unsigned int number_key);

int oswl_benchmark_end(unsigned int module_id,
                        unsigned int line_number,
                        char* function_name,
                        unsigned int number_key);                                   

void oswl_benchmark_iterator_entry(unsigned int module_id,int attr,void (*dump_record)(oswl_benchmark_out_t* record,void* user_data),void* user_data);
void oswl_benchmark_enable(unsigned int module_id);
void oswl_benchmark_disable(unsigned int module_id);
int oswl_benchmark_hash_count();
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif