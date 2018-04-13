#define ERR_NLT  101        //No Language Type
#define ERR_FE   102        //Fork Error
#define ERR_CRE  103        //Chroot Error
#define ERR_MFFE 104        //Mkfifo Errori
#define ERR_OFFE 105        //Open FIFO Error
#define ERR_PTE  106        //Ptrace Error
#define ERR_ER   107        //Execv Error
#define ERR_OFE  108        //Open File Error

#define RS_CE    201        //Compiel error
#define RS_TLE   202        //Time Limit Exceeded
#define RS_MLE   203        //Memory Limit Exceeded
#define RS_RE    204        //Runtime Error
#define RS_OLE   205        //Output Limit Exceeded
#define RS_AC    206        //Accepted
#define RS_PE    207        //Presentation Error

void error(char* error_msg, int error_code){
    _exit(error_code);
}

void return_result(char* result_msg, int result_code){
    _exit(result_code);
}