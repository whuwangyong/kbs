#include "bbs.h"

#define refreshtime     (30)
extern time_t   login_start_time;
extern char     BoardName[];

int   (*func_list_show)();
time_t update_time=0;
int showexplain=0,freshmode=0;
int numf;
int friendmode=0;
int usercounter,real_user_names=0;
int range,page,readplan,num;

struct user_info *user_record[USHM_SIZE];
struct userec *user_data;
extern char MsgDesUid[14]; /* 保存所发msg的目的uid 1998.7.5 by dong */

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

print_title()
{

    docmdtitle((friendmode)?"[好朋友列表]":"[使用者列表]",
               " 聊天[t] 寄信[m] 送讯息[s] 加,减朋友[o,d] 看说明档[→,r] 切换模式 [f] 求救[h]");
    update_endline();
}

print_title2()
{

    docmdtitle((friendmode)?"[好朋友列表]":"[使用者列表]",
               "          寄信[m] 加,减朋友[o,d] 看说明档[→,r] 选择[↑,↓] 求救[h]");
    update_endline();
}

void
update_data(void* data)
{
    if(readplan==YEA)
        return;
   	idle_count++;
    if(time(0)>=update_time+refreshtime*idle_count-1)
    {
        freshmode=1;
        /*Take out by SmallPig*/
        /*否则在执行一些子程式时，画面也会更新*/
        /*                (*func_list_show)();
                        update_endline();
                        move( 3+num-page,0 ); prints( ">");
                        refresh();*/
    }
    set_alarm(refreshtime*idle_count,update_data,NULL);
    UNUSED_ARG(data);
    return;
}


int
print_user_info_title()
{
    char title_str[ 512 ];
    char *field_2 ;

    move(2,0);
    clrtoeol();
    field_2 = "使用者昵称";
    if (real_user_names) field_2 = "真实姓名  ";
    sprintf( title_str,
             /*---	modified by period	2000-10-21	在线用户数可以大于1000的
                     "[44m%s%-12.12s %-16.16s %-16.16s %c %c %-16.16s %5s[m\n",
             ---*/
             "[44m %s%-12.12s %-16.16s %-16.16s %c %c %-15.15s %5s[m\n",
             "编号  ","使用者代号", (showexplain==1)?"好友说明或代号":field_2, "来自", 'P',
             /*(HAS_PERM(currentuser,PERM_SYSOP) ? 'C' : ' ')*/'M', "动态",
#ifdef SHOW_IDLE_TIME
             "时:分" );
#else
"" );
#endif
    prints( "%s", title_str );
    return 0;
}

show_message(msg)
char msg[];
{

    move(BBS_PAGESIZE+3,0);
    clrtoeol();
    if(msg!=NULL)
        prints("[1m%s[m",msg);
    refresh();
}

void swap_user_record(a,b)
int a,b;
{
    struct user_info *c;

    c=user_record[a];
    user_record[a]=user_record[b];
    user_record[b]=c;
}

int full_utmp(struct user_info* uentp,int* count)
{
    if( !uentp->active || !uentp->pid )
    {
        return 0;
    }
    if(!HAS_PERM(currentuser,PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid,currentuser->userid))/*Haohmaru.99.4.24.让隐身者能看见自己*/
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
    /*    struct      user_info *not_good; */

    i2=0;
    if(!friendmode)
    {
	    apply_ulist_addr((APPLY_UTMP_FUNC)full_utmp,(char*)&i2);
    }else {
    	for (i=0;i<nf;i++) {
			if (topfriend[i].uid)
				apply_utmpuid((APPLY_UTMP_FUNC)full_utmp,topfriend[i].uid,(char*)&i2);
	    	}
    }
    range=i2;
    return i2==0?-1:1;
}

char
pagerchar(char* userid1,char* userid2,int pager,int* isfriend)
{
    if (pager&ALL_PAGER) return ' ';
    if (*isfriend==-1)
    	*isfriend=can_override(userid1,userid2);
    if (*isfriend)
    {
        if(pager&FRIEND_PAGER)
            return 'O';
        else
            return '#';
    }
    return '*';
}

char
msgchar( struct user_info *uin,int* isfriend)
{
    if ((uin->pager&ALLMSG_PAGER)) return ' ';
    if (*isfriend==-1)
    	*isfriend=can_override(uin->userid,currentuser->userid);
    if (*isfriend)
    {
        if((uin->pager&FRIENDMSG_PAGER))
            return 'O';
        else
            return '#';
    }
    return '*';
}


int
do_userlist()
{
    int i;
    int fd,len;
    char  user_info_str[256/*STRLEN*2*/],pagec;
    int   override;
    char fexp[30];
    struct user_info uentp;
    /*  _SHOW_ONLINE_USER */
    /* to print on line user to a file */
    /* char online_users[STRLEN+10];

     if(!strcmp(currentuser->userid,"guest")){
     fd=open("onlineulist",O_RDWR|O_TRUNC, 0600);
     if(fd!=-1)
     {
    flock(fd,LOCK_EX);
     	for(i=0; i<range ; i++)
    	 {
        uentp=user_record[i];
        len = sprintf(online_users, " %3d %-12.12s %-24.24s %-20.20s %-17.17s %5.5s\n", i+1,uentp->userid,uentp->username,uentp->from,modestring(uentp->mode, uentp->destuid, 0, (uentp->in_chat ? uentp->chatid : NULL)),uentp->invisible? "#":" ");
         write(fd,online_users,len);
     }
         flock(fd,LOCK_UN);
       close(fd);
     }
}
    */
    /* end of this insertion */

    /*  end of this insertion */
    move(3,0);
    print_user_info_title();

    for(i=0;i<BBS_PAGESIZE&&i+page<range;i++)
    {
        int isfriend;
	isfriend=-1;
	if (user_record[i+page]==NULL) {
		clear();
		prints("[1;31m系统出现Bug,请到Sysop板报告，谢谢![m");
		oflush();
		sleep(10);
		exit(0);
	}
        uentp=*(user_record[i+page]);
        if (!uentp.active||!uentp.pid)
		{
			prints(" %4d 啊,我刚走\n",i+1+page);
			continue;
        }	
        if(!showexplain)
            override=(i+page<numf)||friendmode;
        else
        {
            if((i+page<numf)||friendmode)
                override=myfriend(uentp.uid,fexp);
            else
                override=NA;
        }
        if(readplan==YEA)
        {
            return 0;
        }
        pagec=pagerchar( uentp.userid,currentuser->userid, uentp.pager,&isfriend);
        sprintf( user_info_str,
                 /*---	modified by period	2000-10-21	在线用户数可以大于1000的
                         " %3d%2s%s%-12.12s%s%s %-16.16s%s %-16.16s %c %c %s%-17.17s[m%5.5s\n",
                 ---*/
                 " %4d%2s%s%-12.12s%s%s %-16.16s%s %-16.16s %c %c %s%-16.16s[m%5.5s\n",
                 i+1+page,(override)?(uentp.invisible?"＃":"．"):(uentp.invisible?"＊":""),
                         (override)? "[1;32m":"",uentp.userid
                         ,(override)? "[m":"",(override&&showexplain)?"[1;31m":"",
                         (real_user_names) ? uentp.realname:
                         (showexplain&&override)? fexp:uentp.username,(override&&showexplain)?"[m":"",
                         ((/* !DEFINE(currentuser,DEF_HIDEIP) &&*/ (pagec==' ' || pagec=='O') ) || HAS_PERM(currentuser,PERM_SYSOP)) ? uentp.from : "*",/*Haohmaru.99.12.18*/
                         pagec,
                         /*(uentp.invisible ? '#' : ' ')*/msgchar(&uentp,&isfriend),(uentp.invisible==YEA)
                         ?"[34m":"",
                         modestring(uentp.mode, uentp.destuid, 0,/* 1->0 不显示聊天对象等 modified by dong 1996.10.26 */
                                    (uentp.in_chat ? uentp.chatid : NULL)),
#ifdef SHOW_IDLE_TIME
                         idle_str( &uentp ) );
#else
                         "" );
#endif
        clrtoeol();
        prints( "%s", user_info_str );
    }
    return 0 ;
}

int
show_userlist()
{
    char genbuf[5];


    /*    num_alcounter();*/
    /*    if(!friendmode)
                range=count_users;
        else
                range=count_friends;*/
    if(update_time+refreshtime<time(0))
    {
        fill_userlist();
        update_time=time(0);
    }
    if( range==0/*||fill_userlist() == 0 */) {
        move(2,0);
        prints( "没有使用者（朋友）在列表中...\n" );
        clrtobot();
        if(friendmode){
            getdata(BBS_PAGESIZE+3,0,"是否转换成使用者模式 (Y/N)[Y]: ",genbuf,4,DOECHO,NULL,YEA);
            move(BBS_PAGESIZE+3,0);
            clrtobot();
            if(genbuf[0] != 'N' && genbuf[0] != 'n')
            {
                range=num_visible_users();
                page=-1;
                friendmode=NA;
                return 1;
            }
        }else
            pressanykey();
        return -1;
    }
    do_userlist();
    clrtobot();
    return 1;
}

void
t_rusers()
{
    real_user_names = 1;
    t_users();
    real_user_names = 0;
}

int
deal_key(ch,allnum,pagenum)
char ch;
int allnum,pagenum;
{
    char    buf[STRLEN],genbuf[5];
    static  int   msgflag;

    if(msgflag==YEA)
    {
        show_message(NULL);
        msgflag=NA;
    }
    switch(ch)
    {
case 'k': case'K':
        if(!HAS_PERM(currentuser,PERM_SYSOP)&&strcmp(currentuser->userid,
                                         user_record[allnum]->userid))
            return 1;
        if (!strcmp(currentuser->userid, "guest"))
            return 1; /* Leeward 98.04.13 */
        sprintf(buf,"你要把 %s 踢出站外吗 (Yes/No) [N]: ",
                user_record[allnum]->userid);
        move(BBS_PAGESIZE+3,0);
        clrtoeol();
        getdata(BBS_PAGESIZE+3,0,buf,genbuf,4,DOECHO,NULL,YEA);
        if(genbuf[0] != 'Y' && genbuf[0] != 'y')
        {
            return 1;
        }
        if(kick_user(user_record[allnum])==1)
        {
            sprintf(buf,"%s 已被踢出站外",
                    user_record[allnum]->userid);
        }
        else
        {
            sprintf(buf,"%s 无法踢出站外",
                    user_record[allnum]->userid);
        }
        msgflag=YEA;
        break;
case 'h':case 'H':
        show_help( "help/userlisthelp" );
        break;
case 'W':case 'w':
        if(showexplain==1)
            showexplain=0;
        else
            showexplain=1;
        break;
case 't': case'T':
        if(!HAS_PERM(currentuser,PERM_PAGE))
            return 1;
        if(strcmp(currentuser->userid,
                  user_record[allnum]->userid))
            ttt_talk(user_record[allnum]);
        else
            return 1;
        break;
case 'm': case'M':
        if(!HAS_PERM(currentuser,PERM_POST))
            return 1;
        m_send(user_record[allnum]->userid);
        break;
case 'f': case 'F':
        if(friendmode)
            friendmode=NA;
        else
            friendmode=YEA;
        update_time=0;
        break;
case 's': case 'S':
        if( strcmp(user_record[allnum]->userid,"guest") && 
		!HAS_PERM(currentuser,PERM_PAGE))
            return 1;
        if(!canmsg(user_record[allnum]))
        {
            sprintf(buf,"%s 已经关闭讯息呼叫器",
                    user_record[allnum]->userid);
            msgflag=YEA;
            break;
        }
        /* 保存所发msg的目的uid 1998.7.5 by dong*/
        strcpy(MsgDesUid, user_record[allnum]->userid);
        do_sendmsg(user_record[allnum],NULL,0);
        break;
case 'o': case 'O':
        if(!strcmp("guest",currentuser->userid))
            return 0;
        if(addtooverride(user_record[allnum]->userid)
                ==-1)
        {
            sprintf(buf,"%s 已在朋友名单",
                    user_record[allnum]->userid);
        }
        else
        {
            sprintf(buf,"%s 列入朋友名单",
                    user_record[allnum]->userid);
        }
        msgflag=YEA;
        break;
case 'd': case'D':
        if(!strcmp("guest",currentuser->userid))
            return 0;
        /* Leeward: 97.12.19: confirm removing operation */
        sprintf(buf,"你要把 %s 从朋友名单移除吗 (Y/N) [N]: ",
                user_record[allnum]->userid);
        move(BBS_PAGESIZE+3,0);
        clrtoeol();
        getdata(BBS_PAGESIZE+3,0,buf,genbuf,4,DOECHO,NULL,YEA);
        move(BBS_PAGESIZE+3,0);
        clrtoeol();
        if(genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if(deleteoverride(user_record[allnum]->userid)
                ==-1)
        {
            sprintf(buf,"%s 本来就不在朋友名单中",
                    user_record[allnum]->userid);
        }
        else
        {
            sprintf(buf,"%s 已从朋友名单移除",
                    user_record[allnum]->userid);
        }
        msgflag=YEA;
        break;
    default:
        return 0;
    }
    if(friendmode)
        modify_user_mode(FRIEND);
    else
        modify_user_mode(LUSERS);
    if(readplan==NA)
    {
        print_title();
        clrtobot();
        if(show_userlist()==-1)
            return -1;
        if(msgflag){
            show_message(buf);
        }
        update_endline();
    }
    return 1;
}

int
deal_key2(ch,allnum,pagenum)
char ch;
int allnum,pagenum;
{
    char    buf[STRLEN];
    static  int   msgflag;

    if(msgflag==YEA)
    {
        show_message(NULL);
        msgflag=NA;
    }
    switch(ch)
    {
case 'h':case 'H':
        show_help( "help/usershelp" );
        break;
case 'm': case'M':
        if(!HAS_PERM(currentuser,PERM_POST))
            return 1;
        m_send(user_data[allnum-pagenum].userid);
        break;
case 'o': case 'O':
        if(!strcmp("guest",currentuser->userid))
            return 0;
        if(addtooverride(user_data[allnum-pagenum].userid)
                ==-1)
        {
            sprintf(buf,"%s 已在朋友名单",
                    user_data[allnum-pagenum].userid);
            show_message(buf);
        }
        else
        {
            sprintf(buf,"%s 列入朋友名单",
                    user_data[allnum-pagenum].userid);
            show_message(buf);
        }
        msgflag=YEA;
        if(!friendmode)
            return 1;
        break;
case 'W':case 'w':
        if(showexplain==1)
            showexplain=0;
        else
            showexplain=1;
        break;
case 'd': case'D':
        if(!strcmp("guest",currentuser->userid))
            return 0;
        /* Leeward: 97.12.19: confirm removing operation */
        sprintf(buf,"你要把 %s 从朋友名单移除吗 (Y/N) [N]: ",
                user_data[allnum-pagenum].userid);
        move(BBS_PAGESIZE+3,0);
        clrtoeol();
        getdata(BBS_PAGESIZE+3,0,buf,genbuf,4,DOECHO,NULL,YEA);
        move(BBS_PAGESIZE+3,0);
        clrtoeol();
        if(genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if(deleteoverride(user_data[allnum-pagenum].userid)
                ==-1)
        {
            sprintf(buf,"%s 本来就不在朋友名单中",
                    user_data[allnum-pagenum].userid);
            show_message(buf);
        }
        else
        {
            sprintf(buf,"%s 已从朋友名单移除",
                    user_data[allnum-pagenum].userid);
            show_message(buf);
        }
        msgflag=YEA;
        if(!friendmode)
            return 1;
        break;
    default:
        return 0;
    }
    modify_user_mode(LAUSERS);
    if(readplan==NA)
    {
        print_title2();
        move(3,0);
        clrtobot();
        if(Show_Users()==-1)
            return -1;
    }
    return 1;
}

printuent(struct userec *uentp ,char* arg)
{
    static int i ;
    char permstr[10];
    int override;
    char fexp[30];
    char buf[20] = "           ";


    if(uentp == NULL) {
        move(3,0);
        printutitle();
        i = 0 ;
        return 0;
    }
    if( uentp->numlogins == 0 ||
            uleveltochar( permstr, uentp ) == 0 )
        return 0;
    if(i<page||i>=page+BBS_PAGESIZE||i>=range)
    {
        i++;
        if(i>=page+BBS_PAGESIZE||i>=range)
            return QUIT;
        else
            return 0;
    }
    uleveltochar(&permstr,uentp);
    user_data[i-page]=*uentp;
    override=myfriend(searchuser(uentp->userid),fexp);
    /*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    prints(" %5d%2s%s%-14s%s %s%-19s%s  %5d %5d %4s   %-16s\n",i+1,
#else
    if(HAS_PERM(currentuser,PERM_ADMINMENU))
        sprintf(buf, "%5d %5d", uentp->numlogins, uentp->numposts);
    prints(" %5d%2s%s%-14s%s %s%-19s%s  %11s %4s   %-16s\n", i+1,
#endif
           (override)?"．":"",
           (override)?"[32m":"",uentp->userid,(override)?"[m":"",
           (override&&showexplain)?"[31m":"",
#if defined(ACTS_REALNAMES)
           uentp->realname,
#else
           (override&&showexplain)?fexp:uentp->username,
#endif
           (override&&showexplain)?"[m":"",
#ifdef _DETAIL_UINFO_
           uentp->numlogins, uentp->numposts,
#else
           buf,
#endif
           permstr,
           Ctime(uentp->lastlogin) );
    i++ ;
    usercounter++;
    return 0 ;
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

int
mailto(struct userec *uentp ,char* arg)
{
    char filename[STRLEN];
    int mailmode=(int)arg;

    sprintf(filename,"etc/%s.mailtoall",currentuser->userid);
    if((uentp->userlevel==PERM_BASIC&&mailmode==1)||
            (uentp->userlevel&PERM_POST&&mailmode==2)||
            (uentp->userlevel&PERM_BOARDS&&mailmode==3)||
            (uentp->userlevel&PERM_CHATCLOAK&&mailmode==4))
    {
        mail_file(currentuser->userid,filename,uentp->userid,save_title,0);
    }
    return 1;
}

int mailtoall(mode)
int mode;
{

    return apply_users(mailto,(char*)mode);
}

Show_Users()
{

    usercounter = 0;
    modify_user_mode(LAUSERS );
    printuent((struct userec *)NULL,0) ;

    apply_users(printuent,0);
    clrtobot();
    return 0;
}

setlistrange(i)
int i;
{range=i;}


do_query(star,curr)
int star,curr;
{

    clear();
    if (!user_record[curr]->active) {
            move(t_lines-1,0);
	    prints( "[44m该用户已离线[m");
    } else {
    t_query(user_record[curr]->userid);
    move(t_lines-1,0);
    prints( "[44m聊天[t] 寄信[m] 送讯息[s] 加,减朋友[o,d] 选择使用者[↑,↓] 切换模式 [f] 求救[h][m");
    }
}

do_query2(star,curr)
int star,curr;
{
    t_query(user_data[curr-star].userid);
    move(t_lines-1,0);
    prints( "[44m           寄信[m] 加,减朋友[o,d] 看说明档[→,r] 选择[↑,↓] 求救[h]           [m");
}

Users()
{
    range=allusers();
    modify_user_mode(LAUSERS );
    clear();
    user_data=(struct userec *)calloc(sizeof(struct userec),BBS_PAGESIZE);
    choose(NA,0,print_title2,deal_key2,Show_Users,do_query2);
    clear();
    free(user_data);
    return;
}

int
t_friends()
{
    FILE        *fp;
    char        genbuf[STRLEN];

    modify_user_mode(FRIEND);
    friendmode=YEA;
    sethomefile( genbuf, currentuser->userid,"friends" );
    if ((fp = fopen(genbuf, "r")) == NULL) {
        move( 1, 0 );
        clrtobot();
        prints("你尚未利用 Talk -> Override 设定好友名单，所以...\n");
        pressanykey();
        return 0;
    }
    fclose(fp);
    num_alcounter();
    range=count_friends;
    if( range == 0 ) {
        move( 2, 0 );
        clrtobot();
        prints( "目前无好友上线\n");
        getdata(BBS_PAGESIZE+3,0,"是否转换成使用者模式 (Y/N)[Y]: ",genbuf,4,DOECHO,NULL,YEA);
        move(BBS_PAGESIZE+3,0);
        clrtobot();
        if(genbuf[0] != 'N' && genbuf[0] != 'n')
        {
            range=num_visible_users();
            page=-1;
            friendmode=NA;
            update_time=0;
            choose(YEA,0,print_title,deal_key,show_userlist,do_query);
            clear();
            return 0;
        }
    }else
    {
        update_time=0;
        choose(YEA,0,print_title,deal_key,show_userlist,do_query);
    }
    clear();
    friendmode=NA;
    return 0;
}

int
t_users()
{
    friendmode=NA;
    modify_user_mode(LUSERS);
    range=num_visible_users();
    if( range == 0 ) {
        move( 3, 0 );
        clrtobot();
        prints( "目前无使用者上线\n");
    }
    update_time=0;
    choose(YEA,0,print_title,deal_key,show_userlist,do_query);
    clear();
    return 0;
}

int
choose(update,defaultn,title_show,key_deal,list_show,read)
int update;
int defaultn;
int (*title_show)();
int (*key_deal)();
int (*list_show)();
int (*read)();
{
    int         ch, number,deal;
    readplan=NA;
    (*title_show)();
    func_list_show=list_show;
    set_alarm(0,NULL,NULL);
    if(update==1)
        update_data(NULL);
    page=-1;
    number=0;
    num=defaultn;
    while( 1 ) {
        if( num <= 0 )  num = 0;
        if( num >= range )  num = range - 1;
        if( page < 0||freshmode==1) {
            freshmode=0;
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
            move(3,0);clrtobot();
            if((*list_show)()==-1)
                return -1;
            update_endline();
        }
        if( num < page || num >= page + BBS_PAGESIZE ) {
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
            if((*list_show)()==-1)
                return -1;
            update_endline();
            continue;
        }
        if(readplan==YEA)
        {
            if((*read)(page,num)==-1)
                return num;
        }
        else{
            move( 3+num-page,0 ); prints( ">", number );}
        ch = egetch();
        if(readplan==NA)
            move( 3+num-page,0 ); prints( " " );
        if( ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF )
        {
            if(readplan==YEA)
            {
                readplan=NA;
                move(1,0);
                clrtobot();
                if((*list_show)()==-1)
                    return -1;
                (*title_show)();
                continue;
            }
            break;
        }

        deal=(*key_deal)(ch,num,page);
        if(range==0)
            break;
        if(deal==1)
            continue;
        else if(deal==-1)
            break;
        switch( ch ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
case 'P': case 'b': case Ctrl('B'): case KEY_PGUP:
            if( num == 0 )  num = range - 1;
            else  num -= BBS_PAGESIZE;
            break;
        case ' ':
            if(readplan==YEA){
                if( ++num >= range )  num = 0;
                break;
            }
case 'N': case Ctrl('F'): case KEY_PGDN:
            if( num == range - 1 )  num = 0;
            else  num += BBS_PAGESIZE;
            break;
case 'p': case 'l': case KEY_UP:
            if( num-- <= 0 )  num = range - 1;
            break;
case 'n': case 'j': case KEY_DOWN:
            if( ++num >= range )  num = 0;
            break;
case '$':case KEY_END:
            num = range - 1;       break;
        case KEY_HOME:
            num = 0;       break;
    case '\n': case '\r':
            if( number > 0 ) {
                num = number - 1;
                break;
            }
            /* fall through */
    case 'r': case KEY_RIGHT:
            {
                if(readplan==YEA)
                {
                    if( ++num >= range )  num = 0;
                }
                else
                    readplan=YEA;
                break;
            }
        default:
            ;
        }
        if( ch >= '0' && ch <= '9' ) {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
    }
    set_alarm(0,NULL,NULL);
    return -1 ;
}
