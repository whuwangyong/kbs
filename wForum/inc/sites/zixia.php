<?php

$SiteName="大话西游";

$SiteURL="http://wforum.zixia.net";

$Banner="bar/bar.jpg";

define('OLD_REPLY_STYLE', true); //是否使用传统 telnet re 文方式

define('SMS_SUPPORT', true);

define('AUDIO_CHAT', true);

define ("MAINTITLE","<IMG SRC=\"bar/title.jpg\">");

define("ATTACHMAXSIZE","5242880");
define ("ATTACHMAXTOTALSIZE","20971520");
define("ATTACHMAXCOUNT","20");

define('MYFACEMAXSIZE','102400');

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("斧 头 帮", "[黑帮/系统]"),
    array("大话西游", "[大话/技术]"),
    array("大小分舵", "[地盘/校班]"),
    array("吃喝玩乐", "[美食/休闲]"),
    array("游戏人生", "[游戏/娱乐]"),
    array("吟风弄月", "[感性/文化]"),
    array("西游之路", "[出国/聚会]"),
    array("豆腐西施", "[店面/培训]"),
    array("东城西就", "[省份/城市]"),
    array("大圣取经", "[专业/申请]") 
);

require "dbconn.php";
require "default.php";
?>
