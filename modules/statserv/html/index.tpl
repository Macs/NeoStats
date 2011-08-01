<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<meta http-equiv="Content-Style-Type" content="text/css">

<title>Network Statistics by NeoStats</title>
<style type="text/css">
<!--

BODY {background-color:#444444;color:#ECECEC}
P	{font-family:Verdana, serif;font-size:10pt}

TH	{background-color:#66555F;color:#ECECEC;font-family:Arial, Helvetica, sans-serif;font-size:8pt;font-weight:bold}
TH.secondary	{background-color:#66555F;color:#ECECEC;font-family:Verdana, serif;font-size:10pt;font-weight:normal;text-align:left}
TD.tablebg	{background-color:#000000}
TD.cat	{background-color:#80707F;font-family:Verdana, serif;font-size:12pt}
TD.row1	{background-color:#60707D}
TD.row2	{background-color:#667A80}

SPAN.title	{font-family:Arial, Helvetica, sans-serif;font-size:26pt}
SPAN.cattitle	{font-family:Verdana, serif;font-size:12pt;font-weight:bold}
SPAN.gen	{font-family:Verdana, serif;font-size:10pt}
SPAN.gensmall	{font-family:Verdana, serif;font-size:8pt}
SPAN.courier	{font-family:courier;font-size:10pt}

SELECT {font-family:Verdana;font-size:8pt}
INPUT {font-family:Verdana;font-size:8pt}
SELECT.small	{font-family:"Courier New",courier;font-size:8pt;width:140px}
INPUT.text	{font-family:"Courier New",courier;font-size:8pt;}

INPUT.outsidetable {background-color:#60707D}
INPUT.mainoptiontable {background-color:#60707D}
INPUT.liteoptiontable {background-color:#60707D}

A.forumlinks {font-weight:bold}
A {text-decoration:none}
A:hover {color:#EDF2F2;text-decoration:underline}

HR {height:2px}

//-->
</style>
</head>
<body bgcolor="#444444" text="#ECECEC" link="#EDF2F2" vlink="#DDEDED">

<a name="top"></a>

<table width="98%" cellpadding="0" cellspacing="0" border="0" align="center">
	<tr>
		<td class="tablebg" width="100%"><table width="100%" cellspacing="1" cellpadding="4" border="0">
			<tr>
				<td class="row2"><table width="100%" cellspacing="0" border="0">
					<tr>
						<td><a href="http://www.neostats.net/" target=_NEW>
                          <img border="0" src="http://www.neostats.net/neostats.gif" alt="NeoStats"></a>
                        </td>
						<td align="right" valign="top" bordercolorlight="#000000" bordercolor="#000000"><span class=title>!TITLE!</span></td>
					</tr>
				</table></td>
			</tr>
			<tr>
				<td class="row1"><table width="100%" cellspacing="0" cellpadding="4" border="0">
					<tr>
	 		<td width="90%"><span class="gensmall"><center>
                          <a href="#map">Map</a> <font color="#444444">|</font> 
                          <a href="#netstats">Network Statistics</a> <font color="#444444">|</font>
                          <a href="#daily">Daily Network Statistics</a> <font color="#444444">|</font>
                          <a href="#weekly">Weekly Network Statistics</a> <font color="#444444">|</font>
                          <a href="#monthly">Monthly Network Statistics</a> <font color="#444444">|</font>
                          <a href="#top10chan">Top 10 Channels</a> <font color="#444444"> |</font>
                          <a href="#srvlist">Server List</a> <font color="#444444">|</font> 
                          <a href="#joinchan">Top Join Channels</a> <font color="#444444"> |</font> 
                          <a href="#kickchan">Top Kick Channels</a> <font color="#444444"> |</font> 
                          <a href="#topicchan">Top Topic Channels</a> <font color="#444444"> |</font> 
                          <a href="#srvdet">Server Details</a> 
			  </center></span></td>
					</tr>
				</table></td>
			</tr>
		</table></td>
	</tr>
</table>

<table width="98%" cellspacing="0" cellpadding="4" border="0" align="center">
	<tr>
		<td align="left" valign="bottom">
          <p align="right"><span class="gensmall">Version is: !VERSION!</span></p>
        </td>
	</tr>
</table>

<table width="98%" cellpadding="0" cellspacing="0" border="0" align="center">
	<tr>
		<td class="tablebg"><table width="100%" cellpadding="0"
cellspacing="0" border="1">
			<tr>
	 		<td class="cat" height="30"><span class="cattitle"><b>Network</b>&nbsp;Map</span></td>
			<tr>
				<td class="row1" align="center">
				<a name=map></a>!MAP!
			</td>
			</tr>

			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Current</b>&nbsp;Top Level Domain Statistics</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=tldmap></a>!TLDMAP!
			</td>
			</tr>
	
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Current</b>&nbsp;Network Statistics</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=netstats></a>!NETSTATS!
			</td>
			</tr>
	
			<tr>
	 		 <td class="cat" height="30"><span class="cattitle"><b>Daily</b>&nbsp;Network Statistics</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=daily></a>!DAILYSTATS!
			</td>
			</tr>
			<tr>
	 		 <td class="cat" height="30"><span class="cattitle"><b>Weekly</b>&nbsp;Network Statistics</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=weekly></a>!WEEKLYSTATS!
			</td>
			</tr>
			<tr>
	 		 <td class="cat" height="30"><span class="cattitle"><b>Monthly</b>&nbsp;Network Statistics</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=monthly></a>!MONTHLYSTATS!
			</td>
			</tr>
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Top 10</b>&nbsp;Client Versions</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=clivers></a>!CLIENTSTATS!
			</td>
			</tr>
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Top 10</b>&nbsp;Online Channels</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=top10chan></a>!TOP10CHAN!
			</td>
			</tr>
			<tr>
	 		<td class="cat" height="30"><span class="cattitle"><b>Top 10</b>&nbsp;Join Channels</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=joinchan></a>!TOP10JOIN!
			</td>
			</tr>
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Top 10</b>&nbsp;Kick Channels</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=kickchan></a>!TOP10KICKS!
			</td>
			</tr>
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Top 10</b>&nbsp;Topic Change Channels</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=topicchan></a>!TOP10TOPICS!
			</td>
			</tr>
			<tr>
	 		<td class="cat" height="30"><span class="cattitle"><b>Server</b>&nbsp;List</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=srvlist></a>!SRVLIST!
				(Click on a Server name for more information)
			</td>
			</tr>
			<tr>
	 	 	<td class="cat" height="30"><span class="cattitle"><b>Server</b>&nbsp;Details</span></td>
			</tr>
			<tr>
				<td class="row1" align="center">
				<a name=srvdet></a>!SRVLISTDET!
			</td>
			</tr>
		</table></td>
	</tr>
</table>

<br clear="all" />


</body>
</html>
