#! /bin/sh

BBS_HOME=/home/bbs
INSTALL="//bin/install -c"
TARGET=/home/bbs/bin

echo "This script will install the whole BBS to ${BBS_HOME}..."
echo -n "Press <Enter> to continue ..."
read ans

if [ -d ${BBS_HOME} ] ; then
        echo -n "Warning: ${BBS_HOME} already exists, overwrite whole bbs [N]?"
        read ans
        ans=${ans:-N}
        case $ans in
            [Yy]) echo "Installing new bbs to ${BBS_HOME}" ;;
            *) echo "Abort ..." ; exit ;;
        esac
else
        echo "Making dir ${BBS_HOME}"
        mkdir ${BBS_HOME}
        chown -R bbs ${BBS_HOME}
        chgrp -R bbs ${BBS_HOME}
fi

echo "Setup bbs directory tree ....."
( cd bbshome ; tar cf - * ) | ( cd ${BBS_HOME} ; tar xf - )

chown -R bbs ${BBS_HOME}
chgrp -R bbs ${BBS_HOME}

${INSTALL} -m 770  -s -g bbs -o bbs    bbs        ${TARGET}
${INSTALL} -m 770  -s -g bbs -o bbs    bbs.chatd  ${TARGET}
${INSTALL} -m 4755 -s -g bin -o root   bbsrf      ${TARGET}

cat > ${BBS_HOME}/etc/sysconf.ini << EOF
# comment

BBSHOME         = "/home/bbs"
BBSID           = "NoName"
BBSNAME         = "Never Land BBS"
BBSDOMAIN       = "some.where.on.earth"

#SHOW_IDLE_TIME         = 1
KEEP_DELETED_HEADER     = 0

#BBSNTALKD      = 1
#NTALK          = "/bin/ctalk.sh"
#REJECTCALL     = "/bin/rejectcall.sh"
#GETPAGER       = "getpager"

BCACHE_SHMKEY   = 7813
UCACHE_SHMKEY   = 7912
UTMP_SHMKEY     = 3785
ACBOARD_SHMKEY  = 9013
ISSUE_SHMKEY    = 5002
GOODBYE_SHMKEY  = 5003

IDENTFILE       = "etc/preach"
EMAILFILE       = "etc/mailcheck"
#NEWREGFILE     = "etc/newregister"

PERM_BASIC      = 0x00001
PERM_CHAT       = 0x00002
PERM_PAGE       = 0x00004
PERM_POST       = 0x00008
PERM_LOGINOK    = 0x00010
PERM_DENYPOST   = 0x00020
PERM_CLOAK      = 0x00040
PERM_SEECLOAK   = 0x00080
PERM_XEMPT      = 0x00100
PERM_WELCOME    = 0x00200
PERM_BOARDS     = 0x00400
PERM_ACCOUNTS   = 0x00800
PERM_CHATCLOAK  = 0x01000
PERM_OVOTE      = 0x02000
PERM_SYSOP      = 0x04000
PERM_POSTMASK   = 0x08000
PERM_ANNOUNCE   = 0x10000
PERM_OBOARDS    = 0x20000
PERM_ACBOARD    = 0x40000
UNUSE1          = 0x80000
UNUSE2          = 0x100000
UNUSE3          = 0x200000
UNUSE4          = 0x400000
UNUSE5          = 0x800000
UNUSE6          = 0x1000000
UNUSE7          = 0x2000000
UNUSE8          = 0x4000000
UNUSE9          = 0x8000000
UNUSE10         = 0x10000000
UNUSE11         = 0x20000000

PERM_ADMENU    = PERM_ACCOUNTS , PERM_OVOTE , PERM_SYSOP,PERM_OBOARDS,PERM_WELCOME,PERM_ANNOUNCE
AUTOSET_PERM    = PERM_CHAT, PERM_PAGE, PERM_POST, PERM_LOGINOK

#include "etc/menu.ini"
EOF

cat > ${BBS_HOME}/etc/menu.ini << EOF
#---------------------------------------------------------------------
%S_MAIN






[34m"" -[1;34m _ ..[0m[36m-- -"[1;36m ''- - -.[36m _. ._. ._.[0m[1;34m. _ _. _[34m.- --" [36m'- --._ _ _.[1;36m '. ._ [34m _ .-.-[0m[34m.."[m
                                                                        
                                                        O                      
                 ,                                    o         ,/            
              .//                                   o         ///         
          ,.///;,   ,;/                                   ,,////.   ,/     [1;32m  )[m
         o;;;;;:::;///                              '.  o:::::::;;///    [1;32m ( ([m
        >;;...::::;\\\                                <::::::::;;\\\     [1;32m(  )[m
          ''\\\\\'" ''\                                 ''::::::' \\     [1;32m )([0;32m\[1m([m  
             '\\                                            '\\           [1;32m()[0;32m/[1;32m))[m
                                                               '\         [1;32m\([0;32m|[1m/[m
                                                                          [1;32m()[m[32m|[1m([m
                                                                           [1;32m([m[32m|[1m)[m
                                                                           [1;32m\[m[32m|[1m/[m
%

#---------------------------------------------------------------------
%menu TOPMENU
title        0, 0, "主選單"
screen       3, 0, S_MAIN
!M_EGROUP    10, 28, 0,         "EGroup",    "E) 分類討論區"
!M_MAIL      0, 0, 0,           "Mail",      "M) 處理信箋
!M_TALK      0, 0, 0,           "Talk",      "T) 上鵲橋    Talk  "
!M_INFO      0, 0, PERM_BASIC,  "Info",      "I) 工具箱   Xyz-1  "
!M_SYSINFO   0, 0, 0,           "Config",    "C) 系統資訊 Xyz-2  "
@LeaveBBS    0, 0, 0,           "GoodBye",   "G) 說拜拜          "
@Announce    0, 0, 0,           "0Announce", "0) 精華公佈欄      "
!M_TIN       0, 0, 0,           "1TIN"    ,  "1) 文章彙整   TIN  "
#@ExecGopher  0, 0, 0,           "2Gopher",  "2) 資料查詢 Gopher "
!M_ExceMJ    0, 0, PERM_POST,   "Services",  "S) 網路相關服務與 Game"
#@SetHelp    0, 0, 0,           "Help",      "H) 輔助畫面設定    "
!M_ADMIN     0, 0, PERM_ADMENU,"Admin",      "A) 系統管理功能表  "
%
#---------------------------------------------------------------------
%S_TIN



                                                                        
                                                                        
                                                                       
                                                                            
                            [1;33m讓你享受資料豐富的世界[m
                          ┌───────────┐
                          │◤                  ◥│
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │◣                  ◢│
                          └───────────┘
                                                                        
                                                                
                                                                
%

#---------------------------------------------------------------------
%menu M_TIN
title        0, 0, "Tin 選單"
screen       3, 0, S_TIN
@ExecTin     14, 28, PERM_POST, "1TIN",       "1) 文章彙整   TIN  "
@EGroups     0, 0, 0,           "TINinBBS",   "T) 把 Tin 搬到 BBS"
!..          0, 0, 0,           "Exit",       "E) 回到主選單"
%


#------------------------------------------------------------------
%S_EGROUP






                     ┌─────────────────┐
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
%
#------------------------------------------------------------------
%menu M_EGROUP
title        0, 0, "分類討論區選單"
screen       3, 0, S_EGROUP
@EGroups     10, 25, 0, "0BBS",      "0) BBS 系統 -- [站內]"
@EGroups     0, 0, 0,   "1CCU",      "1) 中正大學 -- [本校]"
@EGroups     0, 0, 0,   "2Campus",   "2) 校園資訊 -- [校園] [資訊]"
@EGroups     0, 0, 0,   "3Computer", "3) 電腦技術 -- [電腦] [系統]"
@EGroups     0, 0, 0,   "4Rec",      "4) 休閒娛樂 -- [休閒] [音樂]"
@EGroups     0, 0, 0,   "5Art",      "5) 文藝學術 -- [文藝] [社交]"
#@EGroups    0, 0, 0,   "6Science",  "6) 學術科學 -- [學科] [語言]"
@EGroups     0, 0, 0,   "6Sports",   "6) 體育健身 -- [運動] [職棒]"
@EGroups     0, 0, 0,   "7Talk",     "7) 談天聊地 -- [談天] [新聞]"
@EGroups     0, 0, 0,   "TINinBBS",  "T) 把 Tin 搬到 BBS"
@PostArticle 0, 0, PERM_POST,   "Post",      "P) 文豪揮筆        "
@BoardsAll   0, 0, 0,   "Boards",    "B) 所有討論區"
@BoardsNew   0, 0, 0,   "New",       "N) 閱讀新文章  New"
!..          0, 0, 0,   "Exit",      "E) 回到主選單"
%
EGROUP0 = "0"
EGROUP1 = "C"
EGROUP2 = "IE"
EGROUP3 = "NR"
EGROUP4 = "am"
EGROUP5 = "enij"
EGROUP6 = "rs"
EGROUP7 = "xf"
EGROUPT = "*"
#------------------------------------------------------------------
%S_MAIL






                          ┌───────────┐
                          │◤                  ◥│
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │                      │
                          │◣                  ◢│
                          └───────────┘
%

#------------------------------------------------------------------
%menu M_MAIL
title            0, 0, "處理信箋選單"
screen           3, 0, S_MAIL
@ReadNewMail     11, 29, 0,             "NewMail", "N) 覽閱新信箋"
@ReadMail        0, 0, 0,               "ReadMail","R) 覽閱全部信箋"
@SendMail        0, 0, PERM_POST,       "SendMail","S) 丟小紙條"
@GroupSend       0, 0, PERM_POST,       "Gsend",   "G) 寄信給一群人"
@SetFriends      0, 0, PERM_BASIC,      "Override","O)┌設定好友名單"
@OverrideSend    0, 0, PERM_POST,       "Osend",   "O)└寄信給好友名單"
@SetMailList     0, 0, PERM_POST,       "Makelist","M)┌設定寄信名單   "
@ListSend        0, 0, PERM_POST,       "Lsend",   "L)└寄給設定的名單 "
@SendNetMail     0, 0, PERM_POST,       "Internet","I) 飛鴿傳書"
!..              0, 0, 0,               "Exit",    "E) 回到主選單"
%
#------------------------------------------------------------------
%S_TALK






  ,-*~,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,'~*-,._.,
                                                
                                                
                                                
                                                
                                                
                                        
                                                
                                                
                                                
                                                
                                                
                                                        
  ,-*~,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,'~*-,._.,
%

#------------------------------------------------------------------
%menu M_TALK
title        0, 0, "上鵲橋選單"
screen       3, 0,   S_TALK
@ShowFriends 10, 27,    PERM_BASIC, "Friends",  "F) 夜探好友   Friend"
@ShowLogins  0, 0,      0,          "Users",    "U) 環顧四方    Users"
@QueryUser   0, 0,      PERM_BASIC, "Query",    "Q) 查詢網友    Query"
@Talk        0, 0,      PERM_POST,  "Talk",     "T) 鵲橋細語     Talk"
@SetPager    0, 0,      PERM_BASIC, "Pager",    "P) 想一個人靜靜 Page"
@SendMsg     0, 0,      PERM_POST,  "SendMsg",  "S) 送訊息給別人"
@SetFriends  0, 0,      PERM_BASIC, "Override", "O) 設定好友名單"
@EnterChat   0, 0,      PERM_BASIC, "1Chat",    "1) 夜貓子客棧"
#@EnterChat   0, 0,      PERM_BASIC, "2Chat",    "2) 夜貓子客棧"
#@EnterChat   0, 0,      PERM_BASIC, "3Relay",   "3) 快哉亭    "
#@EnterChat   0, 0,      PERM_BASIC, "4Chat",    "4) 老大聊天室"
@ExecIrc     0, 0,      PERM_POST,  "IRC",      "I) 星際會談      IRC"
#@ListLogins 0, 0,      PERM_CHAT,  "List",     "L) 上站使用者簡表"
@Monitor     0, 0,      PERM_CHAT,  "Monitor",  "M) 探視民情"
@RealLogins  0, 0,      PERM_SYSOP, "Nameofreal","N) 上站使用者真實姓名"
!..          0, 0, 0,               "Exit",     "E) 回到主選單"
%

#------------------------------------------------------------------
%S_INFO






                          �歈�������������������������
                          ��                        ��
                          ��                        ��
                          ��                        ��
                          ��                        ��
                          ��                        ��
                          ��                        ��
                          ��                        ��
                          ��                        ��                    ,--,/
                          ��                        �礜w╮    ╭─╮_ ___/ /\|
                          ��                        ��  ╰──╯ ,:( )__,_)  ~
                          ��                        ��         //  //   L==;
                          �裺�������������������������         '   \     | ^
 ,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*
%

#------------------------------------------------------------------
%menu M_INFO
title        0, 0, "工具箱選單"
screen       3, 0, S_INFO
@FillForm    11, 29, 0,             "FillForm",  "F) 註冊詳細個人資料"
@SetInfo     0, 0,      PERM_BASIC, "Info",      "I) 設定個人資料"
@UserDefine  0, 0,      PERM_BASIC, "Userdefine","U) 個人參數設定"
#@OffLine     0, 0,      PERM_BASIC, "OffLine",   "O) 自殺~~~~~"
#@EditSig    0, 0,      PERM_POST,  "Signature", "S) 刻印(簽名檔) Sig."
#@EditPlan   0, 0,      PERM_POST,  "QueryEdit", "Q) 擬個人說明檔 Plan"
@EditUFiles  0, 0,      PERM_POST,  "WriteFiles","W) 編修個人檔案"
@SetCloak    0, 0,      PERM_SYSOP, "Cloak",     "C) 隱身術"
@SetHelp     0, 0,      PERM_BASIC, "Help",      "H) 輔助畫面設定"
!..          0, 0, 0,               "Exit",      "E) 回到主選單"
%

#------------------------------------------------------------------
%S_SYSINFO






         ●■■■■■■■■■■■■■■■■■■■■■■■■■■■■●
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ■                                                        ■
         ●■■■■■■■■■■■■■■■■■■■■■■■■■■■■●
                                                                        
%
#------------------------------------------------------------------
%menu M_SYSINFO
title        0, 0, "系統資訊選單"
screen       3, 0, S_SYSINFO
@ShowLicense 10, 28, 0,        "License",   "L) 使用執照      "
@ShowVersion 0, 0, 0,          "Copyright", "C) 智慧權資訊    "
@Notepad     0,0,0,            "Notepad",   "N) 看看留言板"    
@ShowDate    0, 0, 0,          "Date",      "D) 目前時刻     Date "
@DoVote      0, 0, PERM_BASIC, "Vote",      "V) 公民投票     Vote "
@VoteResult  0, 0, PERM_BASIC, "Results",   "R) 選情報導          "
@MailAll     0, 0, PERM_SYSOP, "MailAll",   "M) 寄信給所有人      "
@ExecBBSNet  0, 0, PERM_POST,  "BBSNet",    "B) 穿梭銀河(老馬識途)"  
@ShowWelcome 0, 0, 0,          "Welcome",   "W) 進站畫面      "
@SpecialUser 0, 0, PERM_POST,  "Users",     "U) 合格公民列表  "
#@ExecViewer  0, 0, 0,         "0Announce", "0) 本站公佈欄 (舊版系統)"
!..          0, 0, 0,          "Exit",      "E) 回到主選單"
%

#---------------------------------------------------------------------------
%S_ExceMJ






                     ┌─────────────────┐
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
                     │                                  │
 ,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*
%
#------------------------------------------------------------------
%menu M_ExceMJ
title        0, 0, "網路Game大拼盤  "
screen       3, 0, S_ExceMJ
@WWW        12,29, 0,           "WWW",         "W) 文字模式的 WWW"
@ExecGopher  0, 0, 0,           "Gopher",      "G) 資料查詢 Gopher "
@ExecMJ      0, 0, PERM_CLOAK, "Mujang",       "M) 網路麻將"
@ExecBIG2    0, 0,  0,          "Big2",        "B) 老二最大  "
@ExecCHESS   0, 0,  0,          "Chess",       "C) 決戰象棋  "
!..          0, 0,  0,          "Exit",        "E) 回到主選單"
%

#------------------------------------------------------------------
%S_ADMIN






                                                                        
                                                                             
                                                                             
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
%

#------------------------------------------------------------------
%menu M_ADMIN
title        0, 0, "系統維護選單"
screen       3, 0, S_ADMIN
@CheckForm   9, 28,PERM_SYSOP,  "Register",    "R) 設定使用者註冊資料"
@ModifyInfo  0, 0, PERM_SYSOP,  "Info",        "I) 修改使用者資料"
@ModifyLevel 0, 0, PERM_SYSOP,  "Level",       "L) 更改使用者的權限"
@KickUser    0, 0, PERM_SYSOP,  "Kick",        "K) 將使用者踢出本系統"
@DelUser     0, 0, PERM_ACCOUNTS,"DeleteUser", "D) 砍掉使用者帳號"
@OpenVote    0, 0, PERM_OVOTE,  "Vote",        "V) 舉行系統投票"
@NewBoard    0, 0, PERM_OBOARDS,"NewBoard",    "N) 開啟一個新的討論區"
@EditBoard   0, 0, PERM_ANNOUNCE,"ChangeBoard","C) 修改討論區說明與設定
@DelBoard    0, 0, PERM_OBOARDS,"BoardDelete","B) 砍掉一個無用的討論區"
@SetTrace    0, 0, PERM_SYSOP,  "Trace",       "T) 設定是否記錄除錯資訊"
@CleanMail   0, 0, PERM_SYSOP,  "MailClean",   "M) 清除所有讀過的私人信件"
@Announceall 0, 0, PERM_SYSOP,  "Announceall", "A) 對所有人廣播"
@EditSFiles  0, 0, PERM_WELCOME,"Files",       "F) 編輯系統檔案"
!..          0, 0, 0,           "Exit",        "E) 回到主選單"
%

#------------------------------------------------------------------


EOF

echo "Install is over...."
