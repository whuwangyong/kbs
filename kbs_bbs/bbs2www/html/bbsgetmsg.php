<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		$ret=bbs_getwebmsg($srcid,$msgbuf,$srcutmpnum);
?>
<meta http-equiv='Refresh' content='60; url=bbsgetmsg.php'>
<meta http-equiv="pragma" content="no-cache"><style type="text/css">
A {color: #0000FF}
</style>
<?php
        if ($ret)
		{
?>
<bgsound src="/sound/msg.wav">
<body style="BACKGROUND-COLOR: #f0ffd0">
<table width="100%">
<tr><td><?php echo $msgbuf; ?></td>
<td align="right"><a target="f3" href="/cgi-bin/bbs/bbssendmsg?destid=<?php 
echo $srcid; ?>&destutmp=<?php 
echo $srcutmpnum; ?>">[��ѶϢ]</a> <a href="bbsgetmsg.php">[����]</a></td></tr></table>
<script language=javascript>if (parent.viewfrm)	parent.viewfrm.rows = "18,*,20";
</script>
<?php
        } else {
		//no msg
?><script language=javascript>if (parent.viewfrm)	parent.viewfrm.rows = "0,*,20";
</script>
<?php
		}
		html_normal_quit();
    }
?>