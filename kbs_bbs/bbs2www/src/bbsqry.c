/*
 * $Id$
 */
#include "bbslib.h"

char genbuf[1024];

static void flush_buffer(buffered_output_t *output)
{
	*(output->outp) = '\0'; 
	printf("%s", output->buf);
	output->outp = output->buf;
}

static int buffered_output(char *buf, size_t buflen, void *arg)
{
	buffered_output_t *output = (buffered_output_t *)arg;
	if (output->buflen <= buflen)
	{
		output->flush(output);
		printf("%s", buf);
		return 0;
	}
	if ((output->buflen - (output->outp - output->buf) - 1) <= buflen) 
		output->flush(output);
	strncpy(output->outp, buf, buflen); 
	output->outp += buflen;

	return 0;
}

int show_user_plan(userid)
    char userid[IDLEN];
{
    char pfile[STRLEN];
	int fd;

    sethomefile(pfile, userid, "plans");
    if ((fd = open(pfile, O_RDONLY, 0644)) < 0) {
        hprintf("[36m没有个人说明档[m\n");
    	printf("</pre>\n");
        return 0;
    } else {
		size_t filesize;
		char *ptr;
		const int outbuf_len = 4096;
		buffered_output_t out;

        hprintf("[36m个人说明档如下：[m\n");
    	printf("</pre>\n");
		if (flock(fd, LOCK_EX) == -1)
			return 0;
		BBS_TRY
		{
			if (safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED,
						(void **)&ptr, &filesize) == 0)
			{
				flock(fd, LOCK_UN);
				BBS_RETURN(0);
			}
			if ((out.buf = (char *)malloc(outbuf_len)) == NULL)
			{
				end_mmapfile((void *)ptr, filesize, -1);
				flock(fd, LOCK_UN);
				BBS_RETURN(0);
			}
			out.outp = out.buf;
			out.buflen = outbuf_len;
			out.output = buffered_output;
			out.flush = flush_buffer;
			output_ansi_html(ptr, filesize, &out,NULL);
			free(out.buf);
		}
		BBS_CATCH
		{
		}
		BBS_END end_mmapfile((void *)ptr, filesize, -1);
		flock(fd, LOCK_UN);
        return 1;
    }
}

int t_printstatus(struct user_info *uentp, int *arg, int pos)
{
    if (uentp->invisible == 1) {
        if (!HAS_PERM(currentuser, PERM_SEECLOAK))
            return COUNT;
    }
    (*arg)++;
    if (*arg == 1)
        strcpy(genbuf, "目前在站上，状态如下：\n");
    if (uentp->invisible)
        strcat(genbuf, "[32m隐身中   [m");
    else {
        char buf[80];

        sprintf(buf, "[1m%s[m ", modestring(uentp->mode, uentp->destuid, 0,   /* 1->0 不显示聊天对象等 modified by dong 1996.10.26 */
                                              (uentp->in_chat ? uentp->chatid : NULL)));
        strcat(genbuf, buf);
    }
    if ((*arg) % 8 == 0)
        strcat(genbuf, "\n");
    UNUSED_ARG(pos);
    return COUNT;
}

void display_user(char *userid)
{
    char uident[STRLEN], *newline;
    int tuid = 0;
    int exp, perf;
    struct user_info uin;
    char qry_mail_dir[STRLEN];
    char planid[IDLEN + 2];
    char permstr[10];
    char exittime[40];
    time_t exit_time, temp;
    int logincount, seecount;
    struct userec *lookupuser;
    uinfo_t *ui;
    uinfo_t guestui;

    printf("</center><pre>\n");
    strcpy(uident, strtok(userid, " "));
    if (!(tuid = getuser(uident, &lookupuser))) {
        printf("用户 [%s] 不存在.", userid);
        http_quit();
    }
    ui = getcurruinfo();
    if (ui == NULL) {
        ui = &guestui;
        ui->in_chat = 0;
    }
    ui->destuid = tuid;

    setmailfile(qry_mail_dir, lookupuser->userid, DOT_DIR);

    exp = countexp(lookupuser);
    perf = countperf(lookupuser);
    /*---	modified by period	2000-11-02	hide posts/logins	---*/
    hprintf("%s (%s) 共上站 %d 次，发表过 %d 篇文章", lookupuser->userid, lookupuser->username, lookupuser->numlogins, lookupuser->numposts);
    strcpy(planid, lookupuser->userid);
    if ((newline = strchr(genbuf, '\n')) != NULL)
        *newline = '\0';
    seecount = 0;
    logincount = apply_utmp(t_printstatus, 10, lookupuser->userid, &seecount);
    /* 获得离线时间 Luzi 1998/10/23 */
    exit_time = get_exit_time(lookupuser->userid, exittime);
    if ((newline = strchr(exittime, '\n')) != NULL)
        *newline = '\0';

    if (exit_time <= lookupuser->lastlogin) {
        if (logincount != seecount) {
            temp = lookupuser->lastlogin + ((lookupuser->numlogins + lookupuser->numposts) % 100) + 60;
            strcpy(exittime, ctime(&temp));     /*Haohmaru.98.12.04.让隐身用户看上去离线时间比上线时间晚60到160秒钟 */
            if ((newline = strchr(exittime, '\n')) != NULL)
                *newline = '\0';
        } else
            strcpy(exittime, "因在线上或非常断线不详");
    }
    hprintf("\n上次在  [%s] 从 [%s] 到本站一游。\n离线时间[%s] ", wwwCTime(lookupuser->lastlogin), ((lookupuser->lasthost[0] == '\0') ? "(不详)" : lookupuser->lasthost), exittime);
    uleveltochar(&permstr, lookupuser);
    hprintf("信箱：[[5m%2s[m] 生命力：[%d] 身份: [%s]%s\n",
            (check_query_mail(qry_mail_dir) == 1) ? "信" : "  ", compute_user_value(lookupuser), permstr, (lookupuser->userlevel & PERM_SUICIDE) ? " (自杀中)" : "。");

    if ((genbuf[0]) && seecount) {
        hprintf("%s", genbuf);
        printf("\n");
    }
    show_user_plan(planid);
    printf("<br><br><a href=\"bbspstmail?userid=%s&title=没主题\">[写信问候]</a> ", lookupuser->userid);
    printf("<a href=\"/bbssendmsg.php?destid=%s\">[发送讯息]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfadd?userid=%s\">[加入好友]</a> ", lookupuser->userid);
    printf("<a href=\"bbsfdel?userid=%s\">[删除好友]</a>", lookupuser->userid);
    printf("<hr>");
    printf("</center>\n");

    ui->destuid = 0;
}

int main()
{
    char userid[14];

    init_all();
    strsncpy(userid, getparm("userid"), 13);
    printf("<center>");
    printf("%s -- 查询网友<hr color=green>\n", BBSNAME);
    if (userid[0] == 0) {
        printf("<form action=bbsqry>\n");
        printf("请输入用户名: <input name=userid maxlength=12 size=12>\n");
        printf("<input type=submit value=查询用户>\n");
        printf("</form><hr>\n");
        http_quit();
    }
    display_user(userid);
    http_quit();
}
