#include "bbs.h"

int calcboard(struct boardheader * bh, void * arg)
{
    char fn[80];
    int fd, total, i;
    struct stat buf;
    struct flock ldata;
    struct fileheader * ptr1;
    char * ptr;
    int size=sizeof(struct fileheader);
    setbdir(0, fn, bh->filename);
    if ((fd = open(fn, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return 0;      /* 创建文件发生错误*/
    }
    fstat(fd, &buf);
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    fcntl(fd, F_SETLKW, &ldata);
    total = buf.st_size / size;

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return 0;
    }
    ptr1 = (struct fileheader *) ptr;
    for (i = 0; i < total; i++) {
        struct stat st;
        char* p;
        char ffn[80];
        int j;
        size_t fsize;

        setbfile(ffn, bh->filename, ptr1->filename);
        {
            int k,abssize=0,entercount=0,ignoreline=0;
            j = safe_mmapfile(ffn, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &p, &fsize, NULL);
            if(j) {
                k=fsize;
                while(k) {
                    if(k>=3&&*p=='\n'&&*(p+1)=='-'&&*(p+2)=='-'&&*(p+3)=='\n') break;
                    if(*p=='\n') {
                        entercount++;
                        ignoreline=0;
                    }
                    if(k>=5&&*p=='\n'&&*(p+1)=='\xa1'&&*(p+2)=='\xbe'&&*(p+3)==' '&&*(p+4)=='\xd4'&&*(p+5)=='\xda') ignoreline=1;
                    if(k>=2&&*p=='\n'&&*(p+1)==':'&&*(p+2)==' ') ignoreline=2;
                    k--;
                    p++;
                    if(entercount>=4&&!ignoreline)
                        abssize++;
                }
                ptr1->eff_size = abssize;
            }
            end_mmapfile((void*)p, fsize, -1);
        }
        ptr1++;
    }
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return 0;
}



int main(int argc, char ** argv)
{
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();

    apply_boards(calcboard,NULL);
    return 0;
}
