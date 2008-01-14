<?
/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* configuration */
$target = "./ls";
$disasm_lines = 30;
/* configuration */

$tab = @$_GET["tab"];
if (!$tab) $tab=0;
$action = @$_GET["action"];
$input  = @$_GET["input"];
$offset = @$_GET["offset"];
if ($offset<=0||!$offset) $offset = 0;
if (!strncmp($offset, "0x", 2))
	sscanf($offset, "0x%x", $offset);
$offset=(integer)$offset;

function print_disassembly() {
	global $target;
	global $disasm_lines;
	global $offset;
	global $tab;

	$fd = fopen("$target.txt", "r");
	if (!$fd) {
		print "Cannot open dissaembly file\n";
		return;
	}
	$offstr = sprintf("  %x:\t", $offset);
	$lines  = 0;
	$toggle = 0;
	if ($offset>32)
		printf("<a href=\"?offset=%d\">%8x:</a>\t..\n", $offset-32, $offset-32);
	if ($offset!=0)
		printf("<a href=\"?offset=%d\">%8x:</a>\t.\n", $offset-4, $offset-4);
	while(!feof($fd) && $lines<$disasm_lines) {
		$line = fgets($fd);
		if (!$toggle && strstr($line, $offstr)) $toggle=1;
		if ($toggle) {
			$foo = split(":", $line);
			$foo[0] = str_replace(" ", "",$foo[0]);
			sscanf($foo[0], "%x", $ofset);
			comment_for($ofset);
			$line = str_replace($foo[0].":\t", "<a href=\"?offset=$ofset\">".$foo[0]."</a>:\t", chop($line));
			print $line.xref_for($ofset)."\n";
			$lines++;
		}
	}
	fclose($fd);
	if (!$toggle) {
		print "Oops.";
		$count = @$_GET["count"];
		if ($count++<4) {
			$offset++;
			//header("Location: ?offset=$offset&tab=$tab&count=$count");
			print ("<script>location.href=\"?offset=$offset&count=$count&tab=$tab\";</script>");
			print ("Click here to <a href=\"?offset=$offset&count=$count&tab=$tab\">refresh</a>.");
		}
		exit;
	}
	if ($offset !=0) {
		printf("<a href=\"?offset=%d\">%8x:</a>\t..\n", $offset+$lines, $offset+$lines+($lines%4));
	}
}

function is_printable($ch) {
	return ($ch>20 && $ch<126);
}

function hexdump() {
	global $target;
	global $offset;
	#print(".--[ hexdump ]---------- ---- --- -- --  ·");
	$fd = fopen("$target.bin", "r");
	if (!$fd) {
		print "Cannot open binary file";
		return;
	}
	fseek($fd, $offset);
	$buf = fread($fd, 520);
	$foo = unpack("C*", $buf);

	/* pager */
	if ($offset>0)
		print " <a href=\"?offset=".($offset-20)."&tab=hexdump\">up</a>/";
	else 	print " up/";
	print "<a href=\"?offset=".($offset+20)."&tab=hexdump\">down</a>  ";
	if ($offset>0)
		print " <a href=\"?offset=".($offset-500)."&tab=hexdump\">pageup</a>/";
	else    print " pageup/";
	print "<a href=\"?offset=".($offset+500)."&tab=hexdump\">pagedown</a>\n";

	for ($i=0;sizeof($buf)&&$i<count($foo);$i++) {
		if (!($i%20)) {
			if ($i!=0)
			for($j=$i-20;$j<$i;$j++) {
				if (is_printable($foo[$j+1]))
					print(pack("C",$foo[$j+1]));
				else	print ".";
			}
			printf("\n %08x ", $offset+$i);
		}
		if ($foo[$i+1]==0)
			printf("<font color=\"#a0a0a0\">%02x</font> ", $foo[$i+1]);
		else
		if ($foo[$i+1]==255)
			printf("<font color=\"#c0c044\">%02x</font> ", $foo[$i+1]);
		else
			printf("<font color=\"#f07070\">%02x</font> ", $foo[$i+1]);
		if ($i>518) break;
	}
	for($j=$i-20;$j<$i;$j++) {
		if (is_printable(@$foo[$j+1]))
			print(pack("C",@$foo[$j+1]));
		else	print ".";
	}
	
	fclose($fd);
}

function print_info()
{
	global $target;
	global $offset;
	print "; file   $target\n";
}

function print_controls()
{
	global $offset;
	global $tab;
	global $disasm_lines;

	print "<form method=\"get\">";
	print "<input type=\"hidden\" name=\"tab\" value=\"$tab\" />";
	printf("; offset <input value=\"0x%x\" name=\"offset\">\n", $offset);
	print"; block  512\n";
	print"; lines  $disasm_lines\n";
	print "</form>";
}

// XXX dupped
function edit_notes()
{
	global $offset;
	global $target;

	print "<form method=\"get\">";
	print "<input type=\"hidden\" name=\"action\" value=\"edit_notes\">";
	print "<input type=\"hidden\" name=\"offset\" value=\"$offset\">";
	print "<textarea name=\"input\">";
	@require("$target.notes");
	print "</textarea>";
	print "<p align=\"right\"><input type=\"submit\" value=\"Save\"></p>";
	print "</form>";
}

function edit_comment()
{
	global $offset;
	global $target;

	print "<form method=\"get\">";
	print "offset: $offset\n";
	print "<input type=\"hidden\" name=\"action\" value=\"edit_comment\">";
	printf("<input type=\"hidden\" name=\"offset\" value=\"%s\">", $offset);
	$offset=sprintf("%x", $offset);
	print "<textarea name=\"input\">";
	@include("$target.$offset.txt");
	print "</textarea>";
	print "<p align=\"right\"><input type=\"submit\" value=\"Save\"></p>";
	print "</form>";
}

function comment_for($off)
{
	global $target;

	/* big comments */
	$file=sprintf("$target.%x.txt", $off);
	$fd = @fopen($file, "r");
	if ($fd) {
		print "<div style=\"background-color:#101010\">";
		//print "<textarea>";
		while(!feof($fd)) {
			$line = fgets($fd);
			print "; $line";
		}
		fclose($fd);
		//print "</textarea>";
		print "</div>";
	}

	/* database stuff */
	$fd = fopen("$target.db", "r");
	if (!$fd) {
		return;
	}

	$lines = 0;
	while(!feof($fd) && $lines<20) {
		$line = fgets($fd);
		if (!strncmp($line, "comment ", 8)) {
			$foo = split(" ", $line);
			sscanf($foo[1], "%x", $ofset);
			if ($ofset == $off) {
				$comment = substr($line, strlen($foo[0])+strlen($foo[1])+2);
				print "<font color=\"#30c0c0\">; $comment</font>";
			}
		} else
		if (!strncmp($line, "label ", 6)) {
			$foo = split(" ", $line);
			sscanf($foo[1], "%x", $ofset);
			if ($ofset == $off) {
				$label = chop($foo[2]);
				print "$label:\n";
			}
		}
	}
	fclose($fd);
}

function xref_for($off)
{
	global $target;

	/* database stuff */
	$fd = fopen("$target.db", "r");
	if (!$fd) {
		return;
	}

	$lines = 0;
	$xrefs = "";
	while(!feof($fd) && $lines<20) {
		$line = fgets($fd);
		if (!strncmp($line, "xref ", 5)) {
			$foo = split(" ", $line);
			sscanf($foo[1], "%x", $ofset);
			if ($ofset == $off) {
				sscanf($foo[2], "%x", $ofset);
				$comment = substr($line, strlen($foo[0])+strlen($foo[1]));
				$xrefs.=sprintf(" <a href=\"?offset=%d\">%x</a>", $ofset,$ofset);
			}
		}
	}
	fclose($fd);

	if ($xrefs)
		return "    <font color=\"#30d030\">; refs to$xrefs</font>";
	return "";
}

function strings()
{
	global $target;
	$fd = fopen("$target.str", "r");
	if (!$fd) {
		print "Cannot open strings file\n";
		return;
	}

	$lines = 0;
	$xrefs = "";
	while(!feof($fd) && $lines<20) {
		$line = fgets($fd);
		$foo  = split(" ", $line);
		$bar  = "<a href=\"?offset=".$foo[0]."&tab=hexdump\">".$foo[0]."</a>";
		$line = str_replace($foo[0],$bar, $line);
		print $line;
	}
	fclose($fd);
}
function labels()
{
	global $target;
	global $offset;
	global $tab;
	$fd = @fopen("$target.db", "r");
	if (!$fd) {
		print "Cannot open foobase file\n";
		return;
	}

	print "; labels\n";
	?><form method="get"><input type=hidden name=tab value="<?=$tab?>"><input type=hidden name="action" value="add_label"><input type="hidden" name="offset" value="<?=$offset?>"><?
	print ";    add <input name=\"input\">\n";
	$lines = 0;
	while(!feof($fd) && $lines<20) {
		$line = fgets($fd);
		if (!strncmp($line, "label ", 6)) {
			$foo = split(" ", $line);
			sscanf($foo[1], "%x", $ofset);
	
			$label = $foo[2];
			printf("%8s <a href=\"?offset=%d\">".chop($label)."</a> \n", $foo[1], $ofset);
		}
	}
	fclose($fd);
	print "</form>";
}

?>


<? 
/* ACTION MAN! */
if ($action != "") {
if (!strcmp($action,"add_label")) {
	$fd = @fopen("$target.db","a+");
	if ($fd) {
		fwrite($fd, "label $offset $input\n");
		fclose($fd);
		print "<tt style=\"color:green\">Label '$input' added at offset $offset.</tt>\n";
	} else {
		print "<tt style=\"color:red\">Cannot write to the database file.</tt>\n";
	}
} else
if (!strcmp($action,"add_ref")) {
	$fd = @fopen("$target.db","a+");
	if ($fd) {
		fwrite($fd, sprintf("xref %x %s\n", $offset, $input));
		fclose($fd);
		print "<tt style=\"color:green\">Refrence added at offset $offset pointing to $input.</tt>\n";
	} else {
		print "<tt style=\"color:red\">Cannot write to the database file.</tt>\n";
	}
} else
if (!strcmp($action,"add_com")) {
	$fd = @fopen("$target.db","a+");
	if ($fd) {
		fwrite($fd, sprintf("comment %x %s\n", $offset, $input));
		fclose($fd);
		print "<tt style=\"color:green\">Added comment at offset $offset saying '$input'.</tt>\n";
	} else {
		print "<tt style=\"color:red\">Cannot write to the database file.</tt>\n";
	}
} else
if (!strcmp($action,"edit_notes")) {
	$fd = @fopen("$target.notes","w");
	if ($fd) {
		fwrite($fd, $input);
		fclose($fd);
		print "<tt style=\"color:green\">Added comment at offset $offset.</tt>\n";
	} else {
		print "<tt style=\"color:red\">Cannot write to the database file.</tt>\n";
	}
} else
if (!strcmp($action,"edit_comment")) {
	$off = sprintf("%x", $offset);
	$fd = @fopen("$target.$off.txt","w");
	if ($fd) {
		fwrite($fd, $input);
		fclose($fd);
		print "<tt style=\"color:green\">Added comment at offset $offset.</tt>\n";
	} else {
		print "<tt style=\"color:red\">Cannot write to the database file.</tt>\n";
	}
} }

?>
<html>
<head>
<style>
body {

}
input, textarea {
	background-color:#303030;
	color:white;
	border: 0px;
	font-family:courier;
}

textarea {
	background-color:#101010;
	font-size:12px;
	width:600px;
	height:300px;
}

a {
	color:#c0c0c0;
}
a:hover {
	color:#ffffff;
}
</style>
</head>
<body bgcolor="black" text="white">
<tt>
<?
if (!strcmp($tab, "plain")) {
	print "<pre>";
	$fd = fopen("$target.txt", "r");
	if (!$fd) {
		print "Cannot open dissaembly file\n";
		return;
	}
	$offstr="   0:\t";
	$lines = 0;
	$toggle = 0;
	while(!feof($fd)) {
		$line = fgets($fd);
		if (!$toggle && strstr($line, $offstr)) $toggle=1;
		if ($toggle) {
			$foo = split(":", $line);
			$foo[0] = str_replace(" ", "",$foo[0]);
			sscanf($foo[0], "%x", $ofset);
			comment_for($ofset);
			$line = str_replace($foo[0].":\t", "<a href=\"?offset=$ofset\">".$foo[0]."</a>:\t", chop($line));
			print $line.xref_for($ofset)."\n";
			$lines++;
		}
	}
	fclose($fd);
	print "</pre></tt></body></html>\n";
	exit();
}
?>
<table>
<tr>
<td valign="top" width="240" bgcolor="#101010">
<div style="align:center;background-color:#303030"><center><tt>/* radare web frontend */</tt></center></div>
<div class="controls"><pre>

<? print_info(); ?>
<? print_controls(); ?>
<form method="get"><input type=hidden name=tab value="<?=$tab?>"><input type=hidden name="action" value="add_ref"><input type="hidden" name="offset" value="<?=$offset?>">
; references
;    add <input name="input"></form>
<form method="get"><input type=hidden name=tab value="<?=$tab?>"><input type=hidden name="action" value="add_com"><input type="hidden" name="offset" value="<?=$offset?>">
; comments
;    add <input name="input">
;    <a href="?offset=$offset&tab=comment">edit comment</a>
;    <a href="?offset=$offset&tab=notes">notes</a></form>
<? labels(); ?>
</pre></div>
</td>

<td valign="top" style="padding:5"><pre>
<?
if (!strcmp($tab,"hexdump")) {
	print str_replace("strings",
		"<a href=\"?offset=$offset&tab=strings\">strings</a>",
		str_replace("disasm",
		"<a href=\"?offset=$offset\">disasm</a>",
		str_replace("notes",
		"<a href=\"?offset=$offset&tab=notes\">notes</a>",
		str_replace("comment",
		"<a href=\"?offset=$offset&tab=comment\">comment</a>",
		str_replace("plain",
		"<a href=\"?offset=$offset&tab=plain\">plain</a>",
		";   disasm   hexdump   strings   comment   notes   plain\n")))));
	?> <div class="hexdump"><pre><? hexdump(); ?></pre></div> <?
} else
if (!strcmp($tab,"strings")) {
	print str_replace("hexdump",
		"<a href=\"?offset=$offset&tab=hexdump\">hexdump</a>",
		str_replace("disasm",
		"<a href=\"?offset=$offset\">disasm</a>",
		str_replace("comment",
		"<a href=\"?offset=$offset&tab=comment\">comment</a>",
		str_replace("notes",
		"<a href=\"?offset=$offset&tab=notes\">notes</a>",
		str_replace("plain",
		"<a href=\"?offset=$offset&tab=plain\">plain</a>",
		";   disasm   hexdump   strings   comment   notes   plain\n")))));
	?> <div class="hexdump"><pre><? strings(); ?></pre></div> <?
} else
if (!strcmp($tab,"comment")) {
	print str_replace("hexdump",
		"<a href=\"?offset=$offset&tab=hexdump\">hexdump</a>",
		str_replace("disasm",
		"<a href=\"?offset=$offset\">disasm</a>",
		str_replace("strings",
		"<a href=\"?offset=$offset&tab=strings\">strings</a>",
		str_replace("notes",
		"<a href=\"?offset=$offset&tab=notes\">notes</a>",
		str_replace("plain",
		"<a href=\"?offset=$offset&tab=plain\">plain</a>",
		";   disasm   hexdump   strings   comment   notes   plain\n")))));
	?> <div class="hexdump"><pre><? edit_comment(); ?></pre></div> <?
} else
if (!strcmp($tab,"notes")) {
	print str_replace("hexdump",
		"<a href=\"?offset=$offset&tab=hexdump\">hexdump</a>",
		str_replace("disasm",
		"<a href=\"?offset=$offset\">disasm</a>",
		str_replace("strings",
		"<a href=\"?offset=$offset&tab=strings\">strings</a>",
		str_replace("comment",
		"<a href=\"?offset=$offset&tab=comment\">comment</a>",
		str_replace("plain",
		"<a href=\"?offset=$offset&tab=plain\">plain</a>",
		";   disasm   hexdump   strings   comment   notes   plain\n")))));
	?> <div class="hexdump"><pre><? edit_notes(); ?></pre></div> <?
} else {
	print str_replace("strings",
		"<a href=\"?offset=$offset&tab=strings\">strings</a>",
		str_replace("hexdump",
		"<a href=\"?offset=$offset&tab=hexdump\">hexdump</a>",
		str_replace("notes",
		"<a href=\"?offset=$offset&tab=notes\">notes</a>",
		str_replace("comment",
		"<a href=\"?offset=$offset&tab=comment\">comment</a>",
		str_replace("plain",
		"<a href=\"?offset=$offset&tab=plain\">plain</a>",
		";   disasm   hexdump   strings   comment   notes   plain\n")))));
	?><div class="disassembly"><? print_disassembly(); ?></div>
<? } ?>
</pre></td>
</tr></table>
</tt>
</body>
</html>
