<?
require("funcs.jsp");
$data = array ();
$id = $_POST["id"];
$passwd = $_POST["passwd"];
$kick_multi = $_POST["kick_multi"];
$error=-1;
if ($loginok!=1) {
  if ($id!="") {
    if (bbs_checkpasswd($id,$passwd)!=0)
      $loginok=6;
    else {
      $kick=0;
      if ($kick_multi!="")
	$kick=1;
      $error=bbs_wwwlogin($kick);
      if (($error!=0)&&($error!=2)) {
        $loginok=6;
	if ($error==-1) 
		$loginok=4;
      }
      else {
        $loginok=0;
        $num=bbs_getcurrentuinfo($data);
        setcookie("UTMPKEY",$data["utmpkey"]);
        setcookie("UTMPNUM",$num);
        setcookie("UTMPUSERID",$data["userid"]);
	header("Location: /index.html");
	return;
      }
    }
  }
}
else {
	header("Location: /index.html");
	return;
}
?>
<html>

<?
if ($loginok != 1) {
  if ($loginok!=4) {
    if ($loginok==6) {
?>
<body >
<SCRIPT language="javascript">
	alert("�û�������������µ�½��"+
<?
 echo "\"$error\"";
?>
);
	window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?
    return;
  } else {
?>
<body>
<form action="bbslogin.jsp" method="post">
�û�<input class="default" type="text" name="id" maxlength="12" size="8"><br>
����<input class="default" type="password" name="passwd" maxlength="39" size="8"><br>
<input class="button" type="submit" value="��½">
</form> <br>
<?
  }
  }
  else {
?>
<body >
<form name="infoform" action="bbslogin.jsp" method="post">
<input class="default" type="hidden" name="id" maxlength="12" size="8" value=<?
echo "\"$id\"";
?>
><br>
<input class="default" type="hidden" name="passwd" maxlength="39" size="8" value=<?
echo "\"$passwd\"";
?>
><br>
<input class="default" type="hidden" name="kick_multi" value="1" maxlength="39" size="8"><br>
</form> 
<SCRIPT language="javascript">
	if (confirm("���½�Ĵ��ڹ��࣬�Ƿ��߳�����Ĵ��ڣ�"))
		document.infoform.submit();
	else
		window.location = "/index.html";
</SCRIPT>
</body>
</html>
<?
	return;
  }
} else {
  echo "userid:" . $currentuinfo["userid"] . "<br>";
}
echo $loginok . "<br>" . $error . "<br>";
echo $fromhost;

echo "<br>";
foreach ( $_COOKIE as $key => $var ) {
    echo "$key=$var<br>";
}

?>
</body>
</html>
