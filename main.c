#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>


const char* get_dirname(const char* path) {
    const char *set = strrchr(path, '/');
    return set ? ++set : path;
}

long long process_directory(const char *path) {
    DIR *dir = opendir(path);
    
	if (dir == NULL) {
        perror("invalid open dirictory\n");
        exit(EXIT_FAILURE);
    }
    
    const char *dir_name = get_dirname(path);
	
	int mul = (strcmp(dir_name, "mul") == 0);
    int add = (strcmp(dir_name, "add") == 0);
    
	if (!mul && !add) {
        fprintf(stderr, "invalid name of dir: %s.\n", dir_name);
        closedir(dir);
        exit(EXIT_FAILURE);
    }
    
    long long result = add ? 0LL : 1LL;
    struct dirent* in;
    
    while ((in = readdir(dir)) != NULL) {
        if (strcmp(in->d_name, ".") == 0 || strcmp(in->d_name, "..") == 0)
            continue;
        
        char Path_whole[1024];
        snprintf(Path_whole, sizeof(Path_whole), "%s/%s", path, in->d_name);
        
        struct stat st;
        if (stat(Path_whole, &st) == -1) {
            perror("stat error\n");
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            long long sub_res = process_directory(Path_whole);
            if (add) {
                result += sub_res;
			} else {
                result *= sub_res;
			}
        } else if (S_ISREG(st.st_mode)) {

            const char* extance = strrchr(in->d_name, '.');

            if (extance && strcmp(extance, ".txt") == 0) {
                FILE* file = fopen(Path_whole, "r");
                if (!file) {
                    perror("fopen error\n");
                    continue;
                }

                char buffer[1024];

                if (fgets(buffer, sizeof(buffer), file) != NULL) {
                    char *tkn = strtok(buffer, " \t\r\n");

                    while (tkn) {
                        long long number = atoll(tkn);
                        
						if (add) {
                            result += number;
						} else {
                            result *= number;
						}

                        tkn = strtok(NULL, " \t\r\n");
                    }
                }

                fclose(file);
            }
        }
    }
    
    closedir(dir);
    return result;
}

int main(void) {
    DIR *tmp_dir = opendir("tmp");

    if (!tmp_dir) {
        perror("error of open file: tmp");
        return EXIT_FAILURE;
    }
    
    struct dirent* in;
    char sub_dir_path[1024];

    int found = 0;
    
	while ((in = readdir(tmp_dir)) != NULL) {
        if (in->d_type == DT_DIR && strcmp(in->d_name, ".") && strcmp(in->d_name, "..")) {
            if (strcmp(in->d_name, "add") == 0 || strcmp(in->d_name, "mul") == 0) {
                snprintf(sub_dir_path, sizeof(sub_dir_path), "tmp/%s", in->d_name);
                found = 1;
                break;
            }
        }
    }
    closedir(tmp_dir);
    
    if (!found) {
        fprintf(stderr, "dir tmp not in 'add' or 'mul'.\n");
        return EXIT_FAILURE;
    }
    
    long long final_res = process_directory(sub_dir_path);
    
	FILE *fout = fopen("result.txt", "w");
    
	if (!fout) {
        perror("cant open result.txt for write");
        return EXIT_FAILURE;
    }

    fprintf(fout, "%lld", final_res);
    
	fclose(fout);
    
    return EXIT_SUCCESS;
}
