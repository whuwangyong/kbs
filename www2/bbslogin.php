<?php
require("www2-funcs.php");

set_fromhost();
cache_header("nocache");

@$id = $_POST["id"];
@$passwd = $_POST["passwd"];
@$kick_multi = $_POST["kick_multi"];
@$mainurl = $_GET["mainurl"];
if ($mainurl!="") $mainurl=urlencode($mainurl);
if ($id=="") error_alert("�û�������Ϊ��");

if (($id!="guest")&&bbs_checkpasswd($id,$passwd)!=0) error_alert("�û�������������µ�¼��");
$error=bbs_wwwlogin(($kick_multi!="") ? 1 : 0);
switch($error) {
	case 0:
	case 2:
		//normal
		break;
	case -1:
		prompt_multilogin();
		exit;
	case 7:
		error_alert("�Բ���,��ǰλ�ò�������¼�� ID");
	case 3:
		error_alert("���ʺ���ͣ�������ڽ���");
	case 4:
		error_alert("�� ID ����ӭ���Ը� IP ���û�");
	case 5:
		error_alert("��¼����Ƶ��");
	default:
		error_alert("��¼���󣬴���ţ�" . $error);
}

$data = array();
$num=bbs_getcurrentuinfo($data);

if ($data["userid"] != "guest") {
	$wwwparameters = bbs_getwwwparameters();
	setcookie("WWWPARAMS",$wwwparameters,0,"/");
	$currentuser_num=bbs_getcurrentuser($currentuser);
	
	if(!($currentuser["userlevel"]&BBS_PERM_LOGINOK )) {
		$mainurl = "bbsnew.php";
	}
}
setcookie("UTMPKEY",$data["utmpkey"],0,"/");
setcookie("UTMPNUM",$num,0,"/");
setcookie("UTMPUSERID",$data["userid"],0,"/");

if (!defined("STATIC_FRAME")) $target = "frames.php";
else $target = "frames.html";
if ($mainurl!="")
	header("Location: $target?mainurl=" . $mainurl);
else
	header("Location: $target");


function prompt_multilogin() {
	global $id, $passwd, $mainurl;
?>
<html>
<head><meta http-equiv="Content-Type" content="text/html; charset=gb2312" /></head>
<script type="text/javascript">
function cc() {
	if (confirm("���¼�Ĵ��ڹ��࣬�Ƿ��߳�����Ĵ��ڣ�"))
		document.infoform.submit();
	else
		window.location = "index.html";
}
</script>
<body onload="cc()">
<form name="infoform" action="bbslogin.php?mainurl=<?php echo $mainurl; ?>" method="post">
<input type="hidden" name="id" value="<?php echo $id; ?>">
<input type="hidden" name="passwd" value="<?php echo $passwd; ?>">
<input type="hidden" name="kick_multi" value="1">
</form> 
</body>
</html>
<?php
} 
?>