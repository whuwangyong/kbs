/*
 * $Id$
 */
#include "types.h"
#include "bbslib.h"
#include <assert.h>

time_t update_time=0;
int showexplain=0,freshmode=0;
int mailmode,numf;
int friendmode=0;
int usercounter,real_user_names=0;
int range,page,readplan,num;

struct user_info *user_record[USHM_SIZE];
struct userec *user_data;

int loginok=0;
friends_t bbb[MAXREJECTS];
int badnum=0;

struct user_info *u_info;
/*struct UTMPFILE *shm_utmp;*/
/*struct UCACHE *shm_ucache;*/
char fromhost[IPLEN+1];
char parm_name[256][80], *parm_val[256];
int parm_num=0;
int     favbrd_list[FAVBOARDNUM+1];

friends_t fff[200];

int friendnum=0;
int nf;
struct friends_info *topfriend;



int file_has_word(char *file, char *word) {
	FILE *fp;
	char buf[256], buf2[256];
	fp=fopen(file, "r");
	if(fp==0) return 0;
	while(1) {
		bzero(buf, 256);
		if(fgets(buf, 255, fp)==0) break;
		sscanf(buf, "%s", buf2);
		if(!strcasecmp(buf2, word)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int f_append(char *file, char *buf)
{
	FILE *fp;

	fp = fopen(file, "a");
	if(fp == 0)
		return -1;
	fprintf(fp, "%s", buf);
	fclose(fp);

	return 0;
}

time_t file_time(char *file)
{
	struct stat st;

	if (stat(file, &st) == -1)
		return 0;
	return st.st_mtime;
}

struct stat *f_stat(char *file) {
	static struct stat buf;
	bzero(&buf, sizeof(buf));
	if(stat(file, &buf)==-1) bzero(&buf, sizeof(buf));
	return &buf;
}

char *wwwCTime(time_t t) {
	static char s[80];
	sprintf(s, "%24.24s", ctime(&t));
	return s;
}

char *noansi(char *s) {
	static char buf[1024];
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) { 
			if(s[0]==27) {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(!strchr(";[0123456789", s[0])) mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
}

char *nohtml(char *s) {
	char *buf=calloc(strlen(s)+1, 1);
	int i=0, mode=0;
	while(s[0] && i<1023) {
		if(mode==0) {
			if(s[0]=='<') {
				mode=1;
			} else {
				buf[i]=s[0];
				i++;
			}
		} else {
			if(s[0]=='>') mode=0;
		}
		s++;
	}
	buf[i]=0;
	return buf;
}

char *strright(char *s, int len) {
	int l=strlen(s);
	if(len<=0) return "";
	if(len>=l) return s;
	return s+(l-len);
}

char *strcasestr(char *s1, char *s2) {
	int l;
	l=strlen(s2);
	while(s1[0]) {
		if(!strncasecmp(s1, s2, l)) return s1;
		s1++;
	}
	return 0;
}

int strsncpy(char *s1, char *s2, int n) {
	int l=strlen(s2);
	if(n<0) return;
	if(n>l+1) n=l+1;
	strncpy(s1, s2, n-1);
	s1[n-1]=0;
}

char *ltrim(char *s)
{
	char *ptr;
	char *ptr2;

	if (s[0] == '\0')
		return s;
	ptr = ptr2 = s;
	while (*ptr != '\0')
	{
		if ((*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r')
			&& (*ptr != '\n'))
		{
			break;
		}
		ptr++;
	}
	if (ptr == s)
		return s;
	if (*ptr == '\0')
	{
		s[0] = '\0';
		return s;
	}
	while (*ptr != '\0')
	{
		*ptr2 = *ptr;
		ptr++;
		ptr2++;
	}
	*ptr2 = '\0';

	return s;
}

char *rtrim(char *s)
{
	char *ptr;
	char *ptr2;

	if (s[0] == '\0')
		return s;
	ptr = s;
	ptr2 = s + strlen(s) - 1;
	while (ptr2 != ptr)
	{
		if ((*ptr2 == ' ') || (*ptr2 == '\t') || (*ptr2 == '\r')
			|| (*ptr2 == '\n'))
		{
			*ptr2 = '\0';
			ptr2--;
			continue;
		}
		break;
	}

	return s;
}

char *getsenv(char *s) {
        char *t=getenv(s);
        if(t) return t;
        return "";
}

int http_quit() {
	printf("\n</html>\n");
	exit(0);
}

int http_fatal(char *fmt, ...) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, 1023, fmt, ap);
        va_end(ap);
	buf[1023]=0;
 	printf("错误! %s! <br><br>\n", buf);
	printf("<a href=\"javascript:history.go(-1)\">快速返回</a>");
	http_quit();
}

int strnncpy(char *s, int *l, char *s2) {
	strncpy(s+(*l), s2, strlen(s2));
	(*l)+=strlen(s2);
}

int hsprintf(char *s, char *fmt, ...) {
	char buf[1024], ansibuf[80], buf2[80];
	char *tmp;
	int c, bold, m, i, l, len;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	s[0]=0;
	l=strlen(buf);
	len=0;
	bold=0;
	for(i=0; i<l; i++) {
		c=buf[i];
		if(c=='&') {
			strnncpy(s, &len, "&amp;");
		} else if(c=='<') {
			strnncpy(s, &len, "&lt;");
		} else if(c=='>') {
			strnncpy(s, &len, "&gt;");
		} else if(c==27) {
			if(buf[i+1]!='[') continue;
			for(m=i+2; m<l && m<i+24; m++)
				if(strchr("0123456789;", buf[m])==0) break;
			strsncpy(ansibuf, &buf[i+2], m-(i+2)+1);
			i=m;
			if(buf[i]!='m') continue;
			if(strlen(ansibuf)==0) {
				bold=0;
				strnncpy(s, &len, "<font class=\"c37\">");
			}
			tmp=strtok(ansibuf, ";");
			while(tmp) {
				c=atoi(tmp);
				tmp=strtok(0, ";");
				if(c==0) {
					strnncpy(s, &len, "<font class=\"c37\">");
					bold=0;
				}
				if(c>=30 && c<=37) {
					if(bold==1) sprintf(buf2, "<font class=\"d%d\">", c);
					if(bold==0) sprintf(buf2, "<font class=\"c%d\">", c);
					strnncpy(s, &len, buf2);
				}
			}
		} else {
			s[len]=c;
			len++;
		}
	}
	s[len]=0;
}

int hprintf(char *fmt, ...) {
	char buf[8096], buf2[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf2, 1023, fmt, ap);
	va_end(ap);
	hsprintf(buf, "%s", buf2);
	printf("%s", buf);
}

int hhprintf(char *fmt, ...) {
	char buf0[1024], buf[1024], *s, *getparm();
	int len=0;
	int my_link_mode;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1023, fmt, ap);
	va_end(ap);
	buf[1023]=0;
	s=buf;
	my_link_mode=atoi(getparm("my_link_mode"));
	if(my_link_mode==1)  return hprintf("%s", buf);
	if(!strcasestr(s, "http://") && !strcasestr(s, "ftp://") && !strcasestr(s, "mailto:"))
		return hprintf("%s", buf);
	while(s[0]) {
		if(!strncasecmp(s, "http://", 7) || !strncasecmp(s, "mailto:", 7) || !strncasecmp(s, "ftp://", 6)) {
			char *tmp;
			if(len>0) {
				buf0[len]=0;
				hprintf("%s", buf0);
				len=0;
			}
			tmp=strtok(s, "\'\" \r\t)(,;\n");
			if(tmp==0) break;
			if(1) {
				if(strstr(tmp, ".gif") || strstr(tmp, ".jpg") || strstr(tmp, ".bmp")) {
					printf("<img src=\"%s\">", nohtml(tmp));
					tmp=strtok(0, "");
					if(tmp==0) return -1;
					return hhprintf(tmp);
				}
			}
			printf("<a target=\"_blank\" href=\"%s\">%s</a>", nohtml(tmp), nohtml(tmp));
			tmp=strtok(0, "");
			if(tmp==0) return printf("\n");
			return hhprintf(tmp);
		} else {
			buf0[len]=s[0];
			if(len<1000) len++;
			s++;
		}
	}
	return 0;
}

int parm_add(char *name, char *val) {
	int len=strlen(val);
	if(parm_num>=255) http_fatal("too many parms.");
	parm_val[parm_num]=calloc(len+1, 1);
	if(parm_val[parm_num]==0) http_fatal("memory overflow2 %d %d", len, parm_num);
	strsncpy(parm_name[parm_num], name, 78);
	strsncpy(parm_val[parm_num], val, len+1);
	parm_num++;
}

int http_init() {
	char *buf, buf2[1024], *t2, *t3;
	int n;
#ifndef SILENCE
	printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
	printf("<html>\n");
	printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", CHARSET);
#ifndef MY_CSS
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", CSS_FILE);
#else
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", MY_CSS);
#endif
#endif
	n=atoi(getsenv("CONTENT_LENGTH"));
	if(n>5000000) n=5000000;
	buf=calloc(n+1, 1);
	if(buf==0) http_fatal("memory overflow");
	fread(buf, 1, n, stdin);
	buf[n]=0;
	t2=strtok(buf, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
	t2=strtok(buf2, "&");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			__unhcode(t3);
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, "&");
	}
	strsncpy(buf2, getsenv("HTTP_COOKIE"), 1024);
	/*printf("HTTP_COOKIE = %s\n", buf2);*/
	t2=strtok(buf2, ";");
	while(t2) {
		t3=strchr(t2, '=');
		if(t3!=0) {
			t3[0]=0;
			t3++;
			parm_add(trim(t2), t3);
		}
		t2=strtok(0, ";");
	}
#ifdef SQUID_ACCL
	{
		char *ptr,*p;
		ptr=getsenv("HTTP_X_FORWARDED_FOR");
		p=strrchr(ptr,',');
		if (p!=NULL) {
			while(!isdigit(*p)&&*p) p++;
			if (*p) strncpy(fromhost, p , IPLEN);
			else
			  strncpy(fromhost, ptr, IPLEN);
		}
		else
			  strncpy(fromhost, ptr, IPLEN);
		fromhost[IPLEN]=0;
	}
	if (!fromhost[0])
#endif
		strsncpy(fromhost, getsenv("REMOTE_ADDR"), IPLEN);
}

int __to16(char c) {
	if(c>='a'&&c<='f') return c-'a'+10;
	if(c>='A'&&c<='F') return c-'A'+10;
	if(c>='0'&&c<='9') return c-'0';
	return 0;
}

int __unhcode(char *s) {
	int m, n;
	for(m=0, n=0; s[m]!=0; m++, n++) {
		if(s[m]=='+') {
			s[n]=' ';
			continue;
		}
		if(s[m]=='%') {
			s[n]=__to16(s[m+1])*16+__to16(s[m+2]);
			m+=2;
			continue;
		}
		s[n]=s[m];
	}
	s[n]=0;
}

char *getparm(char *var) {
	int n;
	for(n=0; n<parm_num; n++) 
		if(!strcasecmp(parm_name[n], var)) return parm_val[n];
	return "";
}

/* smh_init() 需要修改 */
int shm_init()
{
	resolve_ucache();
	resolve_utmp();
	resolve_boards(); 
	www_data_init();
}

int user_init(struct userec **x, struct user_info **y)
{
	char id[20], num[20];
	int i, uid, key;
	struct UTMPFILE *utmpshm_ptr;

	strsncpy(id, getparm("UTMPUSERID"), 13);
	strsncpy(num, getparm("UTMPNUM"), 12);
	/*printf("utmpuserid = %s\n", id);*/
	/*printf("utmpnum = %s\n", num);*/
	key=atoi(getparm("UTMPKEY"));
	i=atoi(num);

	if (www_user_init(i,id, key, x, y)==0) return 1;
	return 0;
}

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig)
{
	FILE *fp, *fp2;
	char buf3[256], dir[256];
	struct fileheader header;
	int t, i;

	bzero(&header, sizeof(header));
	strcpy(header.owner, id);
	for(i=0; i<100; i++)
	{
		t=time(0)+i;
		sprintf(buf3, "mail/%c/%s/M.%d.A",
				toupper(userid[0]), userid, i+time(0));
		if(!file_exist(buf3))
			break;
	}
	if(i>=99)
		return -1;
	sprintf(header.filename, "M.%d.A", t);
	strsncpy(header.title, title, 60);
	fp = fopen(buf3, "w");
	if(fp == NULL)
		return -2;
	fp2=fopen(file, "r");
	fprintf(fp, "寄信人: %s (%s)\n", id, nickname);
	fprintf(fp, "标  题: %s\n", title);
	fprintf(fp, "发信站: %s (%s)\n", BBSNAME, wwwCTime(time(0)));
	fprintf(fp, "来  源: %s\n\n", ip);
	if(fp2)
	{
		while(fgets(buf3, 256, fp2) != NULL)
		{
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fprintf(fp, "\n--\n");
	sig_append(fp, id, sig);
	fprintf(fp, "\n[1;%dm※ 来源:．%s %s．[FROM: %.20s][m\n",
			31+rand()%7, BBSNAME, NAME_BBS_ENGLISH, ip);
	fclose(fp);
	sprintf(dir, "mail/%c/%s/.DIR", toupper(userid[0]), userid);
	fp=fopen(dir, "a");
	if(fp == NULL)
		return -1;
	fwrite(&header, sizeof(header), 1, fp);
	fclose(fp);
	return 0;
}

int outgo_post2(struct fileheader *fh, char *board, 
				char *userid, char *username, char *title)
{
    FILE *foo;

    if (foo = fopen("innd/out.bntp", "a"))
    {
        fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board,
                fh->filename, userid, username, title);
        fclose(foo);
    }
}

void add_loginfo2(char *filepath, char *board, struct userec *user, int anony)
{
	FILE *fp;
    FILE *fp2;
    int color;
    char fname[STRLEN];

    color=(user->numlogins%7)+31; /* 颜色随机变化 */
    sethomefile( fname, currentuser->userid,"signatures" );
    fp=fopen(filepath,"a");
    if ((fp2=fopen(fname, "r"))== NULL|| /* 判断是否已经 存在 签名档 */
            user->signature==0 || anony==1)
    {
		fputs("\n--\n", fp);
    }
	else
	{ /*Bigman 2000.8.10修改,减少代码 */
        fprintf(fp,"\n");
	}
    /* 由Bigman增加:2000.8.10 Announce版匿名发文问题 */
    if (!strcmp(board,"Announce"))
        fprintf(fp, "[m[%2dm※ 来源:·%s %s·[FROM: %s][m\n",
				color, "BBS "NAME_BBS_CHINESE"站", NAME_BBS_ENGLISH,
                NAME_BBS_CHINESE" BBS站");
    else
        fprintf(fp, "\n[m[%2dm※ 来源:·%s %s·[FROM: %s][m\n",
				color, "BBS "NAME_BBS_CHINESE"站", NAME_BBS_ENGLISH,
				(anony)?NAME_ANONYMOUS_FROM:user->lasthost);

    if (fp2)
		fclose(fp2);
    fclose(fp);
    return;
}

void write_header2(FILE *fp, char *board, char *title, 
					char *userid, char *username, int anony)
{
	if (!strcmp(board,"Announce"))
		fprintf(fp,"发信人: %s (%s), 信区: %s       \n",
				"SYSOP", NAME_SYSOP, board) ;
	else
		fprintf(fp,"发信人: %s (%s), 信区: %s       \n",
				anony ? board : userid,
				anony ? NAME_ANONYMOUS : username,
				board) ;
	fprintf(fp, 
			"标  题: %s\n发信站: %s (%24.24s)\n\n",
			title, "BBS "NAME_BBS_CHINESE"站", Ctime(time(0)));
}

/* fp 		for destfile*/
/* fp2		for srcfile*/
int write_file2(FILE *fp, FILE *fp2)
{
	char buf3[1024];
	char *ptr;

	while(1)
	{
		if(fgets(buf3, sizeof(buf3), fp2) == NULL)
			break;
		if ((ptr = strrchr(buf3, '\r')) != NULL)
		{
			if (*(ptr+1) == '\n')
			{
				*ptr = '\n';
				*(ptr+1) = '\0';
			}
		}
		fprintf2(fp, buf3);
	}
}

/* return value:
// >0		success
// -1		write .DIR failed*/
int
post_article(char *board, char *title, char *file, struct userec *user,
		char *ip, int sig, int local_save, int anony)
{
    struct fileheader post_file ;
    char        filepath[STRLEN], fname[STRLEN];
    char        buf[256];
    int         fd, anonyboard;
    time_t      now;
	FILE *fp, *fp2;

    memset(&post_file,0,sizeof(post_file)) ;
    anonyboard = seek_in_file("etc/anonymous",board); /* 是否为匿名版 */

    /* 自动生成 POST 文件名 */
    now = time(NULL);
    sprintf(fname,"M.%d.A", now) ;
    setbfile( filepath, board, fname );
    while((fd = open(filepath,O_CREAT|O_EXCL|O_WRONLY,0644)) == -1)
	{
        now++;
        sprintf(fname,"M.%d.A",now) ;
        setbfile(filepath, board, fname);
    }
    close(fd) ;
    strcpy(post_file.filename,fname) ;

	anony = anonyboard && anony;
    strncpy(post_file.owner, anony ? board:getcurruserid(), STRLEN);

    if ((!strcmp(board,"Announce"))&&(!strcmp(post_file.owner,board)))
    	strcpy(post_file.owner,"SYSOP");

	fp = fopen(filepath, "w");
	fp2 = fopen(file, "r");
	write_header2(fp, board, title, user->userid, user->username, anony);
	write_file2(fp, fp2);
	fclose(fp2);
	if (!anony)
		addsignature(fp, user, sig);
	fclose(fp);
    add_loginfo2(filepath, board, user, anony); /*添加最后一行*/

    strncpy( post_file.title, title, STRLEN );
    if ( local_save == 1 ) /* local save */
    {
        post_file.filename[ STRLEN - 1 ] = 'L';
        post_file.filename[ STRLEN - 2 ] = 'L';
    }
	else
    {
        post_file.filename[ STRLEN - 1 ] = 'S';
        post_file.filename[ STRLEN - 2 ] = 'S';
        outgo_post2(&post_file, board, user->userid, user->username, title);
    }

    setbfile( buf, board, DOT_DIR);

    /* 在boards版版主发文自动添加文章标记 Bigman:2000.8.12*/
    if (!strcmp(board, "Board")  && !HAS_PERM(currentuser,PERM_OBOARDS)
		&& HAS_PERM(currentuser,PERM_BOARDS) )
    {
        post_file.accessed[0] |= FILE_SIGN;
    }

    if (append_record( buf, &post_file, sizeof(post_file)) == -1)
	{ /* 添加POST信息 到 当前版.DIR */
        sprintf(buf, "posting '%s' on '%s': append_record failed!",
                post_file.title, board);
        report(buf);
		/* 必须把刚刚创建的文件删除才行 */
		unlink(filepath);
        return -1 ;
    }

    sprintf(buf,"posted '%s' on '%s'", post_file.title, board) ;
    report(buf) ; /* bbslog */
    /*      postreport(post_file.title, 1);*/ /*added by alex, 96.9.12*/
    /*if ( !junkboard(board) )
    //{
    //    currentuser->numposts++;
    //}*/
    return now;
}

int sig_append(FILE *fp, char *id, int sig)
{
	FILE *fp2;
	char path[256];
	char buf[100][256];
	int i, total;
	struct userec *x = NULL;

	if(sig<0 || sig>10)
		return -1;
	getuser(id, &x);
	if(x == 0)
		return -1;
	sprintf(path, "home/%c/%s/signatures", toupper(id[0]), id);
	fp2 = fopen(path, "r");
	if(fp2 == 0)
		return -1;
	for(total=0; total<100; total++)
		if(fgets(buf[total], 255, fp2)==0) break;
	fclose(fp2);
	if (sig * 6 >= total)
		return 0;
	for(i=sig*6; i<sig*6+6; i++)
	{
		if(i>=total)
			break;
		fprintf(fp, "%s", buf[i]);
	}
	x->signature = sig;

	return sig;
}

char* anno_path_of(char *board) {
	FILE *fp;
	static char buf[256], buf1[80], buf2[80];
	fp=fopen("0Announce/.Search", "r");
	if(fp==0) return "";
	while(1) {
		if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
		buf1[strlen(buf1)-1]=0;
		if(!strcasecmp(buf1, board)) {
			sprintf(buf, "/%s", buf2);
			return buf;
		}
	}
	fclose(fp);
	return "";
}

int has_BM_perm(struct userec *user, char *board) {
	bcache_t *x;
	char buf[256], *bm;

	x=getbcache(board);
	if(x==0) return 0;
	if(user_perm(user, PERM_BLEVELS)) return 1;
	if(!user_perm(user, PERM_BOARDS)) return 0;
	strcpy(buf, x->BM);
	bm=strtok(buf, ",: ;&()\n");
	while(bm) {
		if(!strcasecmp(bm, user->userid)) return 1;
		bm=strtok(0, ",: ;&()\n");
	}
	return 0;
}

int has_read_perm(struct userec *user, char *board) {  
	int x;	/* 版面不存在返回0, p和z版面返回1, 有权限版面返回1. */

	if(board[0]<=32) return 0;
	x=getbnum(board);
	if(x==0) return 0; 
	return check_read_perm(user,x);
/*	
	if(x->level==0) return 1;
	if(x->level & (PERM_POSTMASK | PERM_NOZAP)) return 1;
	if(!user_perm(user, PERM_BASIC)) return 0;
	if(user_perm(user, x->level)) return 1;
	return 0;
*/
}

int count_mails(char *id, int *total, int *unread) {
        struct fileheader x1;
	char buf[256];
        int n;
        FILE *fp;
	*total=0;
	*unread=0;
		{
			struct userec *x;
			if (getuser(id, &x) == 0)
				return 0;
		}
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(id[0]), id);
        fp=fopen(buf, "r");
        if(fp==0) return -1;
        while(fread(&x1, sizeof(x1), 1, fp)>0) {
                (*total)++;
                if(!(x1.accessed[0] & FILE_READ)) (*unread)++;
        }
        fclose(fp);
	return 0;
}

int setmsgfile(char *buf, char *id)
{
	if (buf == NULL || id == NULL)
		return -1;
	sethomefile(buf, id, SYS_MSGFILE);
	return 0;
}

int setmsgfilelog(char *buf, char *id)
{
	if (buf == NULL || id == NULL)
		return -1;
	sethomefile(buf, id, SYS_MSGFILELOG);
	return 0;
}

extern char MsgDesUid[14];
int send_msg(char *srcid, int srcutmp, char *destid, int destutmp, char *msg)
{
	int i;
	uinfo_t *uin;

	/* 滤掉特殊字符，应该写成一个函数 */
	for(i=0; i<(int)strlen(msg); i++)
		if((0<msg[i] && msg[i]<=27 )|| msg[i]==-1) msg[i]=32;
	strcpy(MsgDesUid, destid);
	if (destutmp == 0)
		uin = t_search(destid, destutmp);
	else
		uin = get_utmpent(destutmp);
	if (uin == NULL)
		return -1;
	if (strcmp(uin->userid, destid))
		return -1;
	if (uin != NULL && uin->mode == WEBEXPLORE)
	{
		char msgbuf[256];
		char msgbak[256];
		char *timestr;
		time_t now;

		now=time(0);
		timestr=ctime(&now)+11;
		*(timestr+8)='\0';
        snprintf(msgbuf, sizeof(msgbuf),
			"[44m[36m%-12.12s[33m(%-5.5s):[37m%-59.59s[m[%dm\033[%dm\n",
			srcid, timestr, msg, getuinfopid()+100, uin->pid+100);
        snprintf(msgbak, sizeof(msgbak), 
			"[44m[0;1;32m=>[37m%-10.10s[33m(%-5.5s):[36m%-59.59s[m[%dm\033[%dm\n",
			destid, timestr, msg, getuinfopid()+100, uin->pid+100);
		if (destutmp == 0)
			destutmp = get_utmpent_num(uin);
		if (send_webmsg(destutmp, uin->userid, srcutmp, srcid, msgbuf) < 0)
			return -1;
		if (store_msgfile(uin->userid, msgbuf) < 0)
			return -2;
		if(strcmp(srcid, uin->userid))
		{
			if (store_msgfile(srcid, msgbak) < 0)
				return -2;
		}
		return 1;
	}
	return sendmsgfunc(uin, msg,2);
}

int count_life_value(struct userec *urec) {
	int i;
	i=(time(0) - urec->lastlogin)/60;
	if(urec->userlevel & PERM_SPECIAL8)
		return (360*24*60-i)/1440;
	if((urec->userlevel & PERM_XEMPT)||!strcasecmp(urec->userid, "guest"))
        	return 999;
	if(urec->numlogins<=3)
		return (45*1440-i)/1440;
	if(!(urec->userlevel & PERM_LOGINOK))
		return (90*1440-i)/1440;
	return (180*1440-i)/1440;
}

int modify_mode(struct user_info *x, int newmode) {
	if(x==0) return;
	x->mode=newmode;
}

int save_user_data(struct userec *x)
{
	int n;

	n = getusernum(x->userid);
	update_user(x, n, 1);
	return 1;
}

int is_bansite(char *ip) {
	FILE *fp;
	char buf3[256];
	fp=fopen(".bansite", "r");
	if(fp==0) return 0;
	while(fscanf(fp, "%s", buf3)>0)
		if(!strcasecmp(buf3, ip)) return 1;
	fclose(fp);
	return 0;
}

int user_perm(struct userec *x, int level) {
	return (x->userlevel & level);
}

int getusernum(char *id)
{
	return searchuser(id);
}

int loadfriend(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/friends", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                friendnum=fread(fff, sizeof(fff[0]), 200, fp);
                fclose(fp);
        }
}

int isfriend(char *id)
{
	return myfriend(searchuser(id), NULL);
}

int loadbad(char *id) {
        FILE *fp;
        char file[256];
        if(!loginok) return;
        sprintf(file, "home/%c/%s/rejects", toupper(id[0]), id);
        fp=fopen(file, "r");
        if(fp) {
                badnum=fread(fff, sizeof(fff[0]), MAXREJECTS, fp);
                fclose(fp);
        }
}

int isbad(char *id) {
        static inited=0;
        int n;
        if(!inited) {
                loadbad(currentuser->userid);
                inited=1;
        }
        for(n=0; n<badnum; n++)
                if(!strcasecmp(id, bbb[n].id)) return 1;
        return 0;
}

int init_all() {
	srand(time(0)*2+getpid());
	chdir(BBSHOME);
	http_init();
	/*seteuid(BBSUID);*/
	/*if(geteuid()!=BBSUID) http_fatal("uid error.");*/
	shm_init();
	loginok=user_init(&currentuser, &u_info);
}

int init_no_http() {
	srand(time(0)+getpid());
	chdir(BBSHOME);
	shm_init();
}

char *void1(unsigned char *s)
{
        int i;
        int flag=0;
        for(i=0; s[i]; i++) {
                if(flag==0) {
                        if(s[i]>=128) flag=1;
                        continue;
                }
                flag=0;
                if(s[i]<32) s[i-1]=32;
        }
        if(flag) s[strlen(s)-1]=0;
        return s;
}

char *sec(char c) {
	int i;
	for(i=0; i<SECNUM; i++) {
		if(strchr(seccode[i], c)) return secname[i][0];
	}
	return "(unknown.)";
}

char *flag_str(int access) {
	static char buf[80];
	char *flag2="";
	strcpy(buf, "  ");
	if(access & FILE_DIGEST) flag2="G";
	if(access & FILE_MARKED) flag2="M";
	if((access & FILE_MARKED) && (access & FILE_DIGEST)) flag2="B";
	sprintf(buf, "%s", flag2);
	return buf;
}

char *flag_str2(int access, int has_read) {
        static char buf[80];
	strcpy(buf, "   ");
        if(loginok) strcpy(buf, "N  ");
        if(access & FILE_DIGEST) buf[0]='G';
        if(access & FILE_MARKED) buf[0]='M';
        if((access & FILE_MARKED) && (access & FILE_DIGEST)) buf[0]='B';
	if(has_read) buf[0]=tolower(buf[0]);
	if(buf[0]=='n') buf[0]=' ';
        return buf;
}

char *userid_str(char *s) {
	static char buf[512];
	char buf2[256], tmp[256], *ptr, *ptr2;
	strsncpy(tmp, s, 255);
	buf[0]=0;
	ptr=strtok(tmp, " ,();\r\n\t");
	while(ptr && strlen(buf)<400) {
		if(ptr2=strchr(ptr, '.')) {
			ptr2[1]=0;
			strcat(buf, ptr);
			strcat(buf, " ");
		} else {
			ptr=nohtml(ptr);
			sprintf(buf2, "<a href=\"bbsqry?userid=%s\">%s</a> ", ptr, ptr);
			strcat(buf, buf2);
		}
		ptr=strtok(0, " ,();\r\n\t");
	}
	return buf;
}

int fprintf2(FILE *fp, char *s) {
	int i, tail=0, sum=0;
	if(s[0]==':' && s[1]==' ' && strlen(s)>79) {
		sprintf(s+76, "..\n");
		fprintf(fp, "%s", s);
		return;
	}
	for(i=0; s[i]; i++) {
		fprintf(fp, "%c", s[i]);
		sum++;
		if(tail) {
			tail=0;
		} else if(s[i]<0) {
			tail=s[i];
		}
		if(sum>=78 && tail==0) {
			fprintf(fp, "\n");
			sum=0;
		}
	}
}

int get_file_ent(char *board, char *file, struct fileheader *x)
{
	char dir[80];

	sprintf(dir, "boards/%s/.DIR", board);
	return search_record(dir, x, sizeof(struct fileheader), cmpname, file);
}

char *getbfroma(char *path) {
        FILE *fp;
	static char buf1[180], buf2[180];
	int len;
	if(path[0]==0) return "";
	path++;
        fp=fopen("0Announce/.Search", "r");
        if(fp==0) return "";
        while(1) {
		bzero(buf1, 80);
		bzero(buf2, 80);
                if(fscanf(fp, "%s %s", buf1, buf2)<=0) break;
                if(buf1[0]) buf1[strlen(buf1)-1]=0;
		if(buf1[0]=='*') continue;
                if(!strncmp(buf2, path, strlen(buf2))) return buf1;
        }
        fclose(fp);
        return "";
}

int set_my_cookie() {
	FILE *fp;
	char path[256], buf[256], buf1[256], buf2[256];
	int my_t_lines=20, my_link_mode=0, my_def_mode=0;
	sprintf(path, "home/%c/%s/.mywww", toupper(currentuser->userid[0]), currentuser->userid);
 	fp=fopen(path, "r");
 	if(fp) {
 		while(1) {
 			if(fgets(buf, 80, fp)==0) break;
 			if(sscanf(buf, "%80s %80s", buf1, buf2)!=2) continue;
 			if(!strcmp(buf1, "t_lines")) my_t_lines=atoi(buf2);
 			if(!strcmp(buf1, "link_mode")) my_link_mode=atoi(buf2);
			if(!strcmp(buf1, "def_mode")) my_def_mode=atoi(buf2);
 		}
 		fclose(fp);
 	}
	sprintf(buf, "%d", my_t_lines);
	setcookie("my_t_lines", buf);
	sprintf(buf, "%d", my_link_mode);
	setcookie("my_link_mode", buf);
	sprintf(buf, "%d", my_def_mode);
	setcookie("my_def_mode", buf);
}

int has_fill_form() {
        FILE *fp;
        int r;
        char userid[256], tmp[256], buf[256], *ptr;
        fp=fopen("new_register", "r");
        if(fp==0) return 0;
        while(1) {
                if(fgets(buf, 100, fp)==0) break;
                r=sscanf(buf, "%s %s", tmp, userid);
                if(r==2) {
                        if(!strcasecmp(tmp, "userid:") && !strcasecmp(userid, currentuser->userid)) {
                                fclose(fp);
                                return 1;
                        }
                }
        }
        fclose(fp);
        return 0;
}


/*
 * Return value:
 *      0       Success
 *     -1       so called "bad arg1"
 *     -2       so called "bad arg2"
 */
int valid_filename(char *file)
{
	/*if (file == NULL)
		return -1;*/
	assert(file);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		/*http_fatal("错误的参数1");*/
		return -1;
	if(strstr(file, "..") || strstr(file, "/"))
		/*http_fatal("错误的参数2");*/
		return -2;
	return 0;
}

/* added by flyriver, 2001.12.17
 * using getcurrusr() instead of using currentuser directly
 */
struct userec *getcurrusr()
{
	return currentuser;
}

struct userec *setcurrusr(struct userec *user)
{
	if (user == NULL)
		return NULL;
	currentuser = user;
	return currentuser;
}

char *getcurruserid()
{
	return currentuser->userid;
}

unsigned int getcurrulevel()
{
	return currentuser->userlevel;
}

int define(unsigned int x)
{
	return x ? currentuser->userdefine&x : 1;
}

time_t get_idle_time(struct user_info *uentp)
{
	return uentp->freshtime;
}


bcache_t *getbcacheaddr()
{
	return bcache;
}

uinfo_t *getcurruinfo()
{
	return u_info;
}

uinfo_t *setcurruinfo(uinfo_t *ui)
{
	u_info = ui;

	return u_info;
}

/* should do some locking before calling this function */
int eat_file_content(int fd, off_t start, off_t len)
{
    struct stat fs;
    char *addr;
    int i, r;

    if (start < 0 || len <= 0)
        return 1;
    if (fstat(fd, &fs) < 0)
        return -1;
    if (start >= fs.st_size)
        return 1;
    addr = mmap(0, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        return -1;
    if (start + len > fs.st_size)
        len = fs.st_size - start;
    r = fs.st_size - start - len;
    for (i = start; i < r; i++)
        addr[start + i] = addr[start + len + i];
    munmap(addr, fs.st_size);
    ftruncate(fd, fs.st_size - len);

	return 0;
}

int count_online()
{
	return get_utmp_number();
}

int count_www()
{
	int i, total=0;
	for(i=0; i<MAXACTIVE; i++)
	{
		if(utmpshm->uinfo[i].mode==WEBEXPLORE)
			total++;
	}
	return total;
}


int get_ulist_length()
{
	return sizeof(user_record)/sizeof(user_record[0]);
}

struct user_info **get_ulist_addr()
{
	return user_record;
}

uinfo_t *get_user_info(int utmpnum)
{

	if (utmpnum < 1 || utmpnum > USHM_SIZE)
		return NULL;
	return &(utmpshm->uinfo[utmpnum-1]);
}

int set_friends_num(int num)
{
	nf = num;
	return nf;
}

int get_friends_num()
{
	return nf;
}

struct friends_info *init_finfo_addr()
{
	topfriend = NULL;
	return topfriend;
}

struct boardheader *getbcache_addr()
{
	return bcache;
}

/* from talk.c */
int
cmpfuid( a,b )
struct friends   *a,*b;
{
    return strcasecmp(a->id,b->id);
}

int
getfriendstr()
{
    extern int nf;
    int i;
    struct friends* friendsdata;
	char filename[STRLEN];

    if(topfriend!=NULL)
        free(topfriend);
    sethomefile( filename, currentuser->userid,"friends" );
    nf=get_num_records(filename,sizeof(struct friends));
    if(nf<=0)
        return 0;
    if(!HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_SYSOP))/*Haohmaru.98.11.16*/
        nf=(nf>=MAXFRIENDS)?MAXFRIENDS:nf;
    friendsdata=(struct friends *)calloc(sizeof(struct friends),nf);
    get_records(filename,friendsdata,sizeof(struct friends),1,nf);
    
    qsort( friendsdata, nf, sizeof( friendsdata[0] ), cmpfuid );/*For Bi_Search*/
    topfriend=(struct friends_info *)calloc(sizeof(struct friends_info),nf);
    for (i=0;i<nf;i++) {
    	topfriend[i].uid=searchuser(friendsdata[i].id);
    	strcpy(topfriend[i].exp,friendsdata[i].exp);
    }
    free(friendsdata);
    return 0;
}

/* from bbs.c */
void 
record_exit_time()   /* 记录离线时间  Luzi 1998/10/23 */
{
    char path[80];
    FILE *fp;
    time_t now;
    sethomefile( path, getcurruserid() , "exit");
    fp=fopen(path, "wb");
    if (fp!=NULL)
    {
        now=time(NULL);
        fwrite(&now,sizeof(time_t),1,fp);
        fclose(fp);
    }
}

/* from list.c */

int set_friendmode(int mode)
{
	friendmode = mode;

	return friendmode;
}

int get_friendmode()
{
	return friendmode;
}

int myfriend(int uid,char* fexp)
{
    extern int  nf;
    int i,found=NA;
    int cmp;
    /*char buf[IDLEN+3];*/

    if(nf<=0)
    {
        return NA;
    }
    for (i=0;i<nf;i++) {
    	if (topfriend[i].uid==uid) {
    		found=YEA;
    		break;
    	}
    }
    if((found)&&fexp)
        strcpy(fexp,topfriend[i].exp);
    return found;
}


int full_utmp(struct user_info* uentp,int* count)
{
    if( !uentp->active || !uentp->pid )
    {
        return 0;
    }
    if(!HAS_PERM(currentuser,PERM_SEECLOAK) && uentp->invisible 
		&& strcmp(uentp->userid,getcurruserid()))/*Haohmaru.99.4.24.让隐身者能看见自己*/
    {
        return 0;
    }
    if(friendmode&&!myfriend(uentp->uid,NULL))
    {
        return 0;
    }
    user_record[*count]=uentp;
    (*count)++;
    return COUNT;
}

int
fill_userlist()
{
    static int i,i2;

    i2=0;
    if(!friendmode)
    {
	    apply_ulist_addr((APPLY_UTMP_FUNC)full_utmp,(char*)&i2);
    }
	else
	{
    	for (i=0;i<nf;i++)
		{
		if (topfriend[i].uid)
			apply_utmpuid((APPLY_UTMP_FUNC)full_utmp,topfriend[i].uid,(char*)&i2);
    	}
    }
    range=i2;
    return i2==0?-1:1;
}

int
countusers(struct userec *uentp ,char* arg)
{
    char permstr[10];

    if(uentp->numlogins != 0&&uleveltochar( permstr, uentp ) != 0)
		return COUNT;
    return 0;
}

int
allusers()
{
	int count;
    if((count=apply_users(countusers,0)) <= 0) {
        return 0;
    }
    return count;
}

int get_favboard(int num)
{
    if(num > 0 && num <= FAVBOARDNUM)
		return favbrd_list[num];
	else
		return -1;
}

int get_favboard_count()
{
	return favbrd_list[0];
}

int add_favboard(char *brdname)
{
	int i;

	if(brdname != NULL && *brdname)
		i = getbnum(brdname);
	else
		return -3; /* err brdname*/
	if (*favbrd_list > FAVBOARDNUM)
		return -2; /* favboard had reach max limit*/
	if( i > 0 && !IsFavBoard(i-1) )
	{
		int llen;
		llen = ++(*favbrd_list);
		favbrd_list[llen] = i-1;

		return llen; /*return current favorite boards count*/
	}

	return -1; /* brdname not found or brdname already in favbrd_list*/
}

/* from bbsfadd.c */
/*int addtooverride2(char *uident, char *exp)
{
    friends_t tmp;
    int  n;
    char buf[STRLEN];

    memset(&tmp,0,sizeof(tmp));
    setuserfile( buf,currentuser->userid, "friends" );
    if((!HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_SYSOP)) &&
            (get_num_records(buf,sizeof(struct friends))>=MAXFRIENDS) )
    {
        return -1;
    }
    if( myfriend( searchuser(uident) , NULL) )
        return -2;
	if (exp == NULL || exp[0] == '\0')
		tmp.exp[0] = '\0';
	else
	{
		strncpy(tmp.exp, exp, sizeof(tmp.exp)-1);
		tmp.exp[sizeof(tmp.exp)-1] = '\0';
	}
    n=append_record(buf,&tmp,sizeof(struct friends));
    if(n!=-1)
        getfriendstr();
    else
        return -3;
    return n;
}*/

/*
int post_file(char *filename, postinfo_t *pi)
{
	FILE *fp;
	FILE *fp2;
    struct fileheader pf;
	char filepath[STRLEN];
	char buf[STRLEN];

	memset(&pf, 0, sizeof(pf));
	if (get_unifile(buf, pi->board, 0) == -1)
		return -1;
	setbfile(filepath, pi->board, buf);
	strcpy(pf.filename, buf);
	strncpy(pf.owner, pi->userid, STRLEN);
	strncpy(pf.title, pi->title, STRLEN);
	pf.accessed[0] = pi->access & 0xFF;
	pf.accessed[1] = (pi->access >> 8) & 0xFF;
	fp = fopen(filepath, "w");
	fp2 = fopen(filename, "r");	
	write_header2(fp, pi->board, pi->title, pi->userid, pi->username,
		   	pi->anony);
	write_file2(fp, fp2);
	fclose(fp2);
	fclose(fp);
    if ( pi->local == 1 )
	{
		pf.filename[ STRLEN - 1 ] = 'L';
		pf.filename[ STRLEN - 2 ] = 'L';
	}
    else
	{
		pf.filename[ STRLEN - 1 ] = 'S';
		pf.filename[ STRLEN - 2 ] = 'S';
		outgo_post2(&pf, pi->board, pi->userid, pi->username, pi->title);
	}
	setbfile(filepath, pi->board, DOT_DIR);
	if (append_record(filepath, &pf, sizeof(pf)) == -1)
   	{
		sprintf(buf, "post file '%s' on '%s': append_record failed!",
				pf.title, pi->board);
		report(buf);
		return -1;
	}
	sprintf(buf,"posted file '%s' on '%s'", pf.title, pi->board);
	report(buf);

	return 0;
}
*/
/*
 * get_unifile() 用于获得一个独一无二的文件.
 *     filename  文件名缓冲区
 *     key       关键字, 可以是版名或者是用户名
 *     mode      == 0, 表示 filename 是发表文章用的文件名
 *                     此时 key 是版名
 *               == 1, 表示 filename 是投递邮件用的文件名
 *                     此时 key 是用户名
 */
int get_unifile(char *filename, char *key, int mode)
{
    int fd;
	char *ip;
    char filepath[STRLEN];
    int now; /* added for mail to SYSOP: Bigman 2000.8.11 */

    now=time(NULL);
    sprintf(filename, "M.%d.A", now) ;
	if (mode == 0)
	{
		if (getbcache(key) == NULL)
			return -1;
		setbfile(filepath, key, filename);
	}
	else
	{
		if (searchuser(key) == 0)
			return -1;
    	setmailfile(filepath, key, filename);
	}
    ip = strrchr(filename, 'A') ;
    while((fd = open(filepath, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1)
   	{
        if(*ip == 'Z')
            ip++,*ip = 'A', *(ip + 1) = '\0' ;
        else
            (*ip)++ ;
		if (mode == 0)
			setbfile(filepath, key, filename);
		else
			setmailfile(filepath, key, filename);
    }
    close(fd) ;

	return 0;
}

/* 获得一个用户的在线用户数目 */
int count_user_online(char *uid)
{
    return apply_utmp(NULL,0,uid,0) ;
}

/* 获得当前用户的 utmp 号 */
int get_curr_utmpent()
{
	return get_utmpent_num(u_info);
}


/* 以下的代码是cgi和php都使用的*/
#define MAX_WWW_GUEST 30000

static struct user_info www_guest_uinfo;
/* WWW GUEST这样做有个同步问题，就是当被清除一个
  GUEST的时候如果正好这个guest刷新了，那么会重写数据结构
  所以，要注意除了key之外的数据如果要变动，必须保证
  在这种情况下不会逻辑混乱。freshtime因为即使被错误的
  client更新了，误差在秒级，而且也不会造成用户问题，所以
  不会出错。但是如果需要更新key，那么就有可能导致下一个
  guest用户的key被错误的覆盖。这个是个问题
  */
struct WWW_GUEST_S {
	int key;
	time_t freshtime;
	time_t logintime;
};

#define MAX_WWW_MAP_ITEM MAX_WWW_GUEST>>5  /* 除以32 */
#define MAX_WWW_GUEST_IDLE_TIME 3600 /* www guest发呆时间设为1小时*/

struct WWW_GUEST_TABLE {
	int use_map[MAX_WWW_MAP_ITEM];
	time_t uptime;
	struct WWW_GUEST_S guest_entry[MAX_WWW_GUEST];
};

static struct WWW_GUEST_TABLE *wwwguest_shm=NULL;

static void longlock(int signo)
{
    bbslog("5system","www_guest lock for so long time!!!.");
    exit(-1);
}

static int www_guest_lock()
{
    int          fd=0;
    fd = open( "www_guest.lock", O_RDWR|O_CREAT, 0600 );
    if( fd < 0 ) {
        return -1;
    }
    signal(SIGALRM,longlock);
    alarm(10);
    if (flock( fd, LOCK_EX ) ==-1)  {
        return -1;
    }
    signal(SIGALRM,SIG_IGN);
    return fd;
}

static void www_guest_unlock(int fd)
{
	flock(fd,LOCK_UN);
	close(fd);
}

static int www_new_guest_entry()
{
	struct public_data* pub=get_publicshm();
	int fd,i,j;
	time_t now;
	if (pub->www_guest_count>=MAX_WWW_GUEST)
		return -1;
	fd = www_guest_lock();
	if (fd==-1) return -1;

    	now = time( NULL );
    	if(( now > wwwguest_shm->uptime + 240 )||(now < wwwguest_shm->uptime-240)) {
        	wwwguest_shm->uptime = now;
        	bbslog( "1system", "WWW guest:Clean guest table");
        	for (i=0;i<MAX_WWW_GUEST;i++) {
        		if (now-wwwguest_shm->guest_entry[i].freshtime<MAX_WWW_GUEST_IDLE_TIME)
        			continue;
        		/*清除use_map*/
        		wwwguest_shm->use_map[i/32]&=1<<(i%32);
			if ((wwwguest_shm->use_map[i/32])&(1<<(i%32)))
				pub->www_guest_count--;
        		/* 清除数据 */
        		bzero(&wwwguest_shm->guest_entry[i],sizeof(struct WWW_GUEST_S));
        	}
    	}
	for (i=0;i< MAX_WWW_MAP_ITEM;i++)
		if (wwwguest_shm->use_map[i]!=0xFFFFFFFF) {
			int map=wwwguest_shm->use_map[i];
			for (j=0;j<32;j++)
				if ((map&1)==0) {
					wwwguest_shm->use_map[i]|=1<<j;
					wwwguest_shm->guest_entry[i*32+j].freshtime=time(0);
					/*避免被kick下去*/
					break;
				}
				else map=map>>1;
			break;
		}
	if (i!=MAX_WWW_MAP_ITEM)
		pub->www_guest_count++;
	www_guest_unlock(fd);
	if (i==MAX_WWW_MAP_ITEM)
		return -1;
	return i*32+j;
}

static int www_free_guest_entry(int idx)
{
	int fd;
	struct public_data* pub=get_publicshm();
	if ((idx<0)||(idx>MAX_WWW_GUEST))
		return -1;
	fd = www_guest_lock();
	wwwguest_shm->use_map[idx/32]&=1<<(idx%32);
	if ((wwwguest_shm->use_map[idx/32])&(1<<(idx%32)))
		pub->www_guest_count--;
	www_guest_unlock(fd);
	return 0;
}

static int resolve_guest_table()
{
	int iscreate;
    	if( wwwguest_shm == NULL ) {
       	 wwwguest_shm = (struct WWW_GUEST_TABLE*)
        	attach_shm( "WWWGUEST_SHMKEY", 4500, sizeof( *wwwguest_shm ),&iscreate );/*attach user tmp cache */
	        if (iscreate) {
			struct public_data* pub=get_publicshm();
			int fd = www_guest_lock();
			if (fd==-1) return -1;
			bzero(wwwguest_shm,sizeof(*wwwguest_shm));
			pub->www_guest_count=0;
			wwwguest_shm->uptime=time(0);
	        	www_guest_unlock(fd);
	    	}
    	}
    	return 0;
}
int www_data_init()
{
	struct userec *guest;
	/* www_guest_info目前先使用一个全局变量来做，这样
	  会导致线程不安全:P   但是对于进程模型的cgi 和php
	  足够了
	  */
	bzero(&www_guest_uinfo,sizeof(www_guest_uinfo));
	www_guest_uinfo.active=YEA;
	www_guest_uinfo.uid=getuser("guest",&guest);
	if (www_guest_uinfo.uid==0)
		return -1;
	www_guest_uinfo.invisible=YEA;
	www_guest_uinfo.pid=1;
	www_guest_uinfo.mode=WEBEXPLORE;
	strcpy(www_guest_uinfo.username,guest->username);
	strcpy(www_guest_uinfo.userid,guest->userid);
	www_guest_uinfo.pager=0;
	strcpy(www_guest_uinfo.realname,guest->realname);
	www_guest_uinfo.utmpkey=0;

	/* destuid 将被用来存放www guest表的入口*/
	www_guest_uinfo.destuid=0;

	if (resolve_guest_table()!=0)
		return -1;
	return 0;
}

int www_user_init(int useridx,char* userid,int key,struct userec **x, struct user_info **y)
{
	/*printf("utmpuserid = %s\n", id);*/
	/*printf("utmpnum = %s\n", num);*/
	if(!strcasecmp(userid, "new"))
		return -1;
	
	if (strcasecmp(userid,"guest")) {
	 	/* 非guest在线用户处理*/
		if(useridx<1 || useridx>=MAXACTIVE) {
			return -1;
		}
		(*y) = get_utmpent(useridx);
		if((strncmp((*y)->from, fromhost, IPLEN))||((*y)->utmpkey != key))
			return -2;
		
		if((((*y)->active == 0))||((*y)->userid[0] == 0)
			||((*y)->mode!=WEBEXPLORE))
			return -3;

		if(strcmp((*y)->userid, userid))
			return -4;
		getuser((*y)->userid, x);

		if(*x==0)
			return -5;
	} else {
	/* guest用户处理 */
		struct WWW_GUEST_S * guest_info;
		if(useridx<1 || useridx>=MAX_WWW_GUEST)
			return -1;
		guest_info=&wwwguest_shm->guest_entry[useridx];
		if (guest_info->key!=key)
			return -2;
		
		strncpy(www_guest_uinfo.from,fromhost,IPLEN);
		www_guest_uinfo.freshtime=guest_info->freshtime;
		www_guest_uinfo.utmpkey=key;
		www_guest_uinfo.destuid=useridx;
		www_guest_uinfo.logintime=guest_info->logintime;

		*y=&www_guest_uinfo;

		getuser("guest", x);
		if (*x == NULL)
			return -1;
	}
	return 0;
}

static int cmpuids2(int unum, struct user_info *urec)
{
		    return (unum == urec->uid);
}

int www_user_login(struct userec* user,int useridx,int kick_multi,char* fromhost,
	struct user_info** ppuinfo,int* putmpent)
{
	int ret;
	char buf[255];
	if(strcasecmp(user->userid, "guest")) {
		struct user_info ui;
		int utmpent;
              time_t t;
		int multi_ret=1;
		int tmp;
		while (multi_ret!=0) {
			int lres;
			int num;
			struct user_info uin;
			multi_ret=multilogin_user(user,useridx);
			if ((multi_ret!=0)&&(!kick_multi))
				return -1;
			if (multi_ret==0) break;
	                if ( !(num=search_ulist( &uin, cmpuids2, useridx) ))
	                        continue;  /* user isn't logged in */
			if (uin.pid==1) {
				clear_utmp(num,useridx);
				continue;
			}
	                if (!uin.active || (kill(uin.pid,0) == -1)) {
				clear_utmp(num,useridx);
	                        continue;  /* stale entry in utmp file */
			}
	/*---	modified by period	first try SIGHUP	2000-11-08	---*/
			lres = kill(uin.pid, SIGHUP);
			sleep(1);
			if(lres)
	/*---	---*/
	                  kill(uin.pid,9);
			clear_utmp(num,useridx);
		}

		if(!HAS_PERM(user, PERM_BASIC))
			return 3;
		if(check_ban_IP(fromhost,buf))
			return 4;
		t=user->lastlogin;
		if(abs(t-time(0))<5)
			return 5;
		user->lastlogin=time(0);
		user->numlogins++;
		strncpy(user->lasthost, fromhost, IPLEN);
		if (!HAS_PERM(user,PERM_LOGINOK) && !HAS_PERM(user,PERM_SYSOP))
		{
			if (strchr(user->realemail, '@')
				&& valid_ident(user->realemail))
			{
				user->userlevel |= PERM_DEFAULT;
				if (HAS_PERM(user,PERM_DENYPOST)/* && !HAS_PERM(currentuser,PERM_SYSOP)*/)
					user->userlevel &= ~PERM_POST;
			}
		}

		memset( &ui, 0, sizeof( struct user_info ) );
    		ui.active = YEA ;
		/* Bigman 2000.8.29 智囊团能够隐身 */
		if( (HAS_PERM(user,PERM_CHATCLOAK)
			|| HAS_PERM(user,PERM_CLOAK)) 
			&& (user->flags[0] & CLOAK_FLAG))
		    	ui.invisible = YEA;
		ui.pager = 0;
		if(DEFINE(user,DEF_FRIENDCALL))
		{
		    ui.pager|=FRIEND_PAGER;
		}
		if(user->flags[0] & PAGER_FLAG)
		{
		    ui.pager|=ALL_PAGER;
		    ui.pager|=FRIEND_PAGER;
		}
		if(DEFINE(user,DEF_FRIENDMSG))
		{
		    ui.pager|=FRIENDMSG_PAGER;
		}
		if(DEFINE(user,DEF_ALLMSG))
		{
		    ui.pager|=ALLMSG_PAGER;
		    ui.pager|=FRIENDMSG_PAGER;
		}
		ui.uid=useridx;
		strncpy( ui.from, fromhost, IPLEN );
		ui.logintime=time(0);	/* for counting user's stay time */
										/* refer to bbsfoot.c for details */
		ui.freshtime = time(0);
		tmp=rand()%100000000;
		ui.utmpkey=tmp;
		ui.mode = WEBEXPLORE;
		strncpy( ui.userid,   user->userid,   20 );
		strncpy( ui.realname, user->realname, 20 );
		strncpy( ui.username, user->username, 40 );
		utmpent = getnewutmpent(&ui) ;
		if (utmpent == -1)
			ret=1;
		else {
			struct user_info* u;
			u = get_utmpent(utmpent);
			u->pid = 1;
			if (addto_msglist(utmpent, user->userid) < 0)
			{
				bbslog("3system","can't add msg:%d %s!!!\n",utmpent,user->userid);
				*ppuinfo=u;
				*putmpent=utmpent;
				ret=2;
			}
			else {
				*ppuinfo=u;
				*putmpent=utmpent;
				ret=0;
			}
		}
	} else {
	/* TODO:alloc guest table*/ 
		int idx=www_new_guest_entry();
		if (idx<0) ret = 5;
		else {
			int tmp=rand()%100000000;
			wwwguest_shm->guest_entry[idx].key=tmp;
			wwwguest_shm->guest_entry[idx].logintime=time(0);
			www_guest_uinfo.logintime=wwwguest_shm->guest_entry[idx].logintime;

			wwwguest_shm->guest_entry[idx].freshtime=time(0);
			www_guest_uinfo.freshtime=wwwguest_shm->guest_entry[idx].freshtime;

			www_guest_uinfo.destuid=idx;
			www_guest_uinfo.utmpkey=tmp;
			*ppuinfo=&www_guest_uinfo;
			*putmpent=-1;
			ret=0;
		}
	}
	
	if ((ret==0)||(ret==2)) {
		snprintf(buf, sizeof(buf), "ENTER ?@%s [www]", fromhost);
		bbslog("1system", "%s",buf);
	}
	return ret;
}

static void setflags(struct userec* u, int mask, int value)
{
    if (((u->flags[0] & mask) && 1) != value) {
        if (value) u->flags[0] |= mask;
        else u->flags[0] &= ~mask;
    }
}
int www_user_logoff(struct userec* user,int useridx,struct user_info* puinfo,int userinfoidx)
{
	int stay=0;
	struct userec *x = NULL;

	stay = abs(time(0) - u_info->logintime);
	/* 上站时间超过 2 小时按 2 小时计 */
	if(stay>7200)
		stay = 7200;
	user->stay+=stay;
	record_exit_time();
	bbslog( "1system", "EXIT: Stay:%3ld (%s)[%d %d]", stay / 60, 
			x->username, get_curr_utmpent(), useridx);
	if (strcasecmp(user->userid,"guest")) {
	    	if(!puinfo->active) return 0;
	    		setflags(user,PAGER_FLAG, (puinfo->pager&ALL_PAGER));

		if((HAS_PERM(user,PERM_CHATCLOAK) || HAS_PERM(user,PERM_CLOAK)))
	        	setflags(user,CLOAK_FLAG, puinfo->invisible);
    	clear_utmp(userinfoidx,useridx);
	} else {
		www_free_guest_entry(puinfo->destuid);
	}
	return 0;
}

time_t set_idle_time(struct user_info *uentp, time_t t)
{
	if (strcasecmp(uentp->userid,"guest"))
		uentp->freshtime = t;
	else {
		int idx;
		idx=uentp->destuid;
		if (idx>=1&&idx<MAX_WWW_GUEST)
			wwwguest_shm->guest_entry[uentp->destuid].freshtime=t;
	}

	return t;
}
