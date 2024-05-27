/***********************************************************************************************************************************************************************
** @file:       hex.c
** @author:     huixuan.li
** @date:       2024-05-27 14:23:49
** @brief:      展示将字节流的16进制值
***********************************************************************************************************************************************************************/
#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <io.h>
#elif defined(__linux__)
#elif defined(__APPLE__)
#else
# error Your operating system is not supported！
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define BUFFER_SIZE 1024

/* 返回码定义 */
enum{ SUCCESS, ARGS_ERROR, HANDLE_ERROR } EXITCODE;

/* 错误信息与帮助信息 */
char ARG_ERROR_INFO[] =
"ERROR! Couldn't recognize argument %s, please use option \"--help\" for instructions\n";
char HELP_INFO[] =
"The program can read a file or input stream and output its hexadecimal value to other file or standrad output, \"-b\" option set the read mode to binary, as follows: \n"
"\t<command> | bytes [-b] [-o <outputfile>]\n"
"\t<inputfile> > bytes [-b] [-o <outputfile>]\n"
"\tbytes <inputfile> [-b] [-o <outputfile>]\n"
"Alternatively, use the bytes command to perform interactive input and output, as shown below:\n"
"\tbytes [-b] [-o <outputfile>]\n";

/* 缓冲区 */
char OUTPUT_FILE_BUFFER[BUFFER_SIZE];

/* 全局参数 */
struct mode{
    bool if_binary;
    bool if_stdin_redirect;
    bool if_stdout_redirect;
} mode = { false,false };

char *input_file_path = NULL;
char *output_file_path = NULL;
FILE *input_file_stream;
FILE *output_file_stream;

/* 读取参数并根据参数修改全局参数 */
void analyze_arguments(int argc, char **argv);

/* 根据全局参数设置输入流 */
void set_input_stream(void);

/* 根据全局参数设置输出流 */
void set_output_stream(void);

/* 进行转化输出 */
void transform(void);

/* 主函数 */
int main(int argc, char *argv[]){
    analyze_arguments(argc, argv);
    set_input_stream();
    set_output_stream();
    transform();
    exit(EXITCODE = SUCCESS);
}


/* 读取参数并根据参数修改全局参数 */
void analyze_arguments(int argc, char **argv){
    // 对参数遍历处理
    for (int i = 1;i < argc;i++){
        char *p = argv[i];
        // 若以-开头,则判定为选项
        if (*p == '-'){
            // -b选项
            if (strcmp(p, "-b") == 0){
                mode.if_binary = true;
            }
            // --help选项
            else if (strcmp(p, "--help") == 0){
                fputs(HELP_INFO, stdout);
                exit(EXITCODE = SUCCESS);
            }
            // -o选项
            else if (strcmp(p, "-o") == 0){
                // 若为参数末尾
                if (i == argc - 1){
                    fprintf(stderr, "Error! Couldn't find outputfile in arguments!\n");
                    exit(EXITCODE = ARGS_ERROR);
                }
                // 若不为参数末尾
                output_file_path = argv[++i];
            }
            // 未知选项
            else{
                fprintf(stderr, ARG_ERROR_INFO, p);
                exit(EXITCODE = ARGS_ERROR);
            }
        }
        // 若参数不以-开头,则判定为输入文件名
        else{
            // 若存在多个文件名则报错
            if (input_file_path != NULL){
                fprintf(stderr, "Error! Multiple input_file set: %s and %s\n", input_file_path, p);
                exit(EXITCODE = ARGS_ERROR);
            }
            input_file_path = p;
        }
    }

    // 检测标准输入输出是否被重定向
    struct stat stat_info;
    // 获取文件描述符的状态信息
    if (fstat(fileno(stdin), &stat_info) == -1){
        fputs("Error! Couldn't get stdin statement!\n", stderr);
        exit(EXITCODE = ARGS_ERROR);
    }
    mode.if_stdin_redirect = !S_ISCHR(stat_info.st_mode);
    // 获取文件描述符的状态信息
    if (fstat(fileno(stdout), &stat_info) == -1){
        fputs("Error! Couldn't get stdin statement!\n", stderr);
        exit(EXITCODE = ARGS_ERROR);
    }
    mode.if_stdout_redirect = !S_ISCHR(stat_info.st_mode);
}

/* 根据全局参数设置输入流 */
void set_input_stream(void){
    // 二进制模式下
    if (mode.if_binary){
        // 指定输入文件时
        if (input_file_path != NULL){
            input_file_stream = fopen(input_file_path, "rb");
            if (input_file_stream == NULL){
                fprintf(stderr, "Error occur in open input_file %s!\n", input_file_path);
                exit(EXITCODE = HANDLE_ERROR);
            }
        }
        // 不指定输入文件时
        else{
            // 以二进制打开输入流
        #if defined(_WIN32) || defined(_WIN64)
            _setmode(_fileno(stdin), _O_BINARY);
        #else
            freopen(NULL, "rb", stdin);
        #endif
            input_file_stream = stdin;
        }
    }
    //非二进制模式下
    else{
        // 指定输入文件时
        if (input_file_path != NULL){
            input_file_stream = fopen(input_file_path, "r");
            if (input_file_stream == NULL){
                fprintf(stderr, "Error occur in open input_file %s!\n", input_file_path);
                exit(EXITCODE = HANDLE_ERROR);
            }
        }
        // 不指定输入文件时
        else{
            input_file_stream = stdin;
        }
    }
}

/* 根据全局参数设置输出流 */
void set_output_stream(void){
    // 指定输出文件时
    if (output_file_path != NULL){
        output_file_stream = fopen(output_file_path, "w");
        if (output_file_stream == NULL){
            fprintf(stderr, "Error occur in open output_file %s, please check whether the path exists!\n", output_file_path);
            exit(EXITCODE = HANDLE_ERROR);
        }
    }
    // 不指定输出文件时
    else{
        output_file_stream = stdout;
    }

    // 重定向标准输入或指定输入文件时设置缓冲区（非交互模式时设置缓冲区）
    if (mode.if_stdin_redirect || input_file_path != NULL){
        if (setvbuf(output_file_stream, OUTPUT_FILE_BUFFER, _IOFBF, BUFFER_SIZE)){
            fprintf(stderr, "ERROR! Couldn't set buffer for output stream!\n");
            exit(EXITCODE = HANDLE_ERROR);
        }
    }
}

/* 进行转化输出 */
void transform(void){
    // 二进制模式
    if (mode.if_binary){
        bool line_begin = true;
        char c;
        // 交互模式
        if (input_file_path == NULL && !mode.if_stdin_redirect){
            while (true){
                c = fgetc(input_file_stream);

                // 若出现错误
                if (ferror(input_file_stream)){
                    fclose(input_file_stream);
                    fclose(output_file_stream);
                    fprintf(stderr, "Unknown error occur in reading!\n");
                    exit(EXITCODE = HANDLE_ERROR);
                }
                // 若超出末尾
                if (feof(input_file_stream)){
                    fclose(input_file_stream);
                    fclose(output_file_stream);
                    return;
                }

            #if defined(_WIN32)|| defined(_WIN64)||defined(__linux__)
                if (line_begin){
                    if (c == '\n'){
                        fprintf(output_file_stream, "0x%02hhX\n", c);
                        line_begin = true;
                    } else{
                        fprintf(output_file_stream, "0x%02hhX", c);
                        line_begin = false;
                    }
                } else{
                    if (c == '\n'){
                        fprintf(output_file_stream, "\t0x%02hhX\n", c);
                        line_begin = true;
                    } else{
                        fprintf(output_file_stream, "\t0x%02hhX", c);
                    }
                }
            #elif defined(__APPLE__)
                if (line_begin){
                    if (c == '\n' || c == '\r'){
                        fprintf(output_file_stream, "0x%02hhX\n", c);
                        line_begin = true;
                    } else{
                        fprintf(output_file_stream, "0x%02hhX", c);
                        line_begin = false;
                    }
                } else{
                    if (c == '\n' || c == '\r'){
                        fprintf(output_file_stream, "\t0x%02hhX\n", c);
                        line_begin = true;
                    } else{
                        fprintf(output_file_stream, "\t0x%02hhX", c);
                    }
                }
            # endif
            }
        }
        // 非交互模式
        else{
            while (true){
                c = fgetc(input_file_stream);

                // 若出现错误
                if (ferror(input_file_stream)){
                    fclose(input_file_stream);
                    fclose(output_file_stream);
                    fprintf(stderr, "Unknown error occur in reading!\n");
                    exit(EXITCODE = HANDLE_ERROR);
                }
                // 若超出末尾
                if (feof(input_file_stream)){
                #if defined(__linux__)
                    // linux下若在控制台打印额外打印换行符
                    if (output_file_path == NULL && !mode.if_stdout_redirect){
                        fprintf(output_file_stream, "\n");
                    }
                #endif
                    fclose(input_file_stream);
                    fclose(output_file_stream);
                    return;
                }

                if (line_begin){
                    fprintf(output_file_stream, "0x%02hhX", c);
                    line_begin = false;
                } else{
                    fprintf(output_file_stream, "\t0x%02hhX", c);
                }
            }
        }

    }
    // 非二进制模式
    else{
        char c;
        bool line_begin = true;

        while (true){
            c = fgetc(input_file_stream);
            // 若出现异常
            if (ferror(input_file_stream)){
                fprintf(stderr, "Unknown error occur in reading!\n");
                fclose(input_file_stream);
                fclose(output_file_stream);
                exit(EXITCODE = HANDLE_ERROR);
            }

            // 若超出末尾
            if (feof(input_file_stream)){
            #if defined(__linux__)
                // 若在控制台展示，linux额外打印换行符
                if (output_file_path == NULL && !mode.if_stdout_redirect){
                    fprintf(output_file_stream, "\n");
                }
            #endif
                fclose(input_file_stream);
                fclose(output_file_stream);
                return;
            }

            if (line_begin){
                if (c == '\n'){
                    fprintf(output_file_stream, "0x%02hhX\n", c);
                    line_begin = true;
                } else{
                    fprintf(output_file_stream, "0x%02hhX", c);
                    line_begin = false;
                }
            } else{
                if (c == '\n'){
                    fprintf(output_file_stream, "\t0x%02hhX\n", c);
                    line_begin = true;
                } else{
                    fprintf(output_file_stream, "\t0x%02hhX", c);
                }
            }
        }
    }
}
