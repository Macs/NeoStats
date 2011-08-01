# NeoStats Perl Module to test Perl Interface
# 
# Copyright 2004 Justin Hammond
#
# You can use this module as a template to code your own. I'll try to describe a lot of it 
# for you.

# First thing that you must ensure happens (ie, first command) is to register this script
# Using the following command:
use Devel::Peek;
import NeoStats::NV;
use Storable;
my $bot;

NeoStats::register( "Test", "1.0", "Test Script 1 description", "setupbot", "shutdownbot");

# Events make up the core communications of NeoStats, here you register the events your 
# Interested in, and the function to call. A optional third arguement ($options) allows you
# to specify optional items such as:
#    Event Flags: 
#	These Determine under what circumstances the event will be called
#	Specify as $option->{flags} = <EVENT_FLAG_???>
#    User Data:
#	Not Currently implemented

NeoStats::hook_event(NeoStats::EVENT_MODULELOAD, "event_moduleload");
NeoStats::hook_event(NeoStats::EVENT_MODULEUNLOAD, "event_moduleunload");
NeoStats::hook_event(NeoStats::EVENT_SERVER, "event_server");
NeoStats::hook_event(NeoStats::EVENT_SQUIT, "event_squit");
NeoStats::hook_event(NeoStats::EVENT_PING, "event_ping");
NeoStats::hook_event(NeoStats::EVENT_PONG, "event_pong");
NeoStats::hook_event(NeoStats::EVENT_SIGNON, "event_signon");
NeoStats::hook_event(NeoStats::EVENT_QUIT, "event_quit");
NeoStats::hook_event(NeoStats::EVENT_NICKIP, "event_nickip");
NeoStats::hook_event(NeoStats::EVENT_KILL, "event_kill");
NeoStats::hook_event(NeoStats::EVENT_GLOBALKILL, "event_globalkill");
NeoStats::hook_event(NeoStats::EVENT_LOCALKILL, "event_localkill");
NeoStats::hook_event(NeoStats::EVENT_SERVERKILL, "event_serverkill");
NeoStats::hook_event(NeoStats::EVENT_BOTKILL, "event_botkill");
NeoStats::hook_event(NeoStats::EVENT_NICK, "event_nick");
NeoStats::hook_event(NeoStats::EVENT_AWAY, "event_away");
NeoStats::hook_event(NeoStats::EVENT_UMODE, "event_umode");
NeoStats::hook_event(NeoStats::EVENT_SMODE, "event_smode");
NeoStats::hook_event(NeoStats::EVENT_NEWCHAN, "event_newchan");
NeoStats::hook_event(NeoStats::EVENT_DELCHAN, "event_delchan");
NeoStats::hook_event(NeoStats::EVENT_JOIN, "event_join");
NeoStats::hook_event(NeoStats::EVENT_PART, "event_part");
NeoStats::hook_event(NeoStats::EVENT_PARTBOT, "event_partbot");
NeoStats::hook_event(NeoStats::EVENT_EMPTYCHAN, "event_emptychan");
NeoStats::hook_event(NeoStats::EVENT_KICK, "event_kick");
NeoStats::hook_event(NeoStats::EVENT_KICKBOT, "event_kickbot");
NeoStats::hook_event(NeoStats::EVENT_TOPIC, "event_topic");
NeoStats::hook_event(NeoStats::EVENT_CMODE, "event_cmode");
NeoStats::hook_event(NeoStats::EVENT_PRIVATE, "event_private");
NeoStats::hook_event(NeoStats::EVENT_NOTICE, "event_notice");
NeoStats::hook_event(NeoStats::EVENT_CPRIVATE, "event_cprivate");
NeoStats::hook_event(NeoStats::EVENT_CNOTICE, "event_cnotice");
NeoStats::hook_event(NeoStats::EVENT_GLOBOPS, "event_globops");
NeoStats::hook_event(NeoStats::EVENT_CHATOPS, "event_chatops");
NeoStats::hook_event(NeoStats::EVENT_WALLOPS, "event_wallops");
NeoStats::hook_event(NeoStats::EVENT_CTCPVERSIONRPL, "event_ctcpversionrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPVERSIONREQ, "event_ctcpversionreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPFINGERRPL, "event_ctcpfingerrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPFINGERREQ, "event_ctcpfingerreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPACTIONREQ, "event_ctcpactionreq");
NeoStats::hook_event(NeoStats::EVENT_CTCPTIMERPL, "event_ctcptimerpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPTIMEREQ, "event_ctcptimereq");
NeoStats::hook_event(NeoStats::EVENT_CTCPPINGRPL, "event_ctcppingrpl");
NeoStats::hook_event(NeoStats::EVENT_CTCPPINGREQ, "event_ctcppingreq");
NeoStats::hook_event(NeoStats::EVENT_DCCSEND, "event_dccsend");
NeoStats::hook_event(NeoStats::EVENT_DCCCHAT, "event_dccchat");
NeoStats::hook_event(NeoStats::EVENT_DCCCHATMSG, "event_dccchatmsg");
NeoStats::hook_event(NeoStats::EVENT_ADDBAN, "event_addban");
NeoStats::hook_event(NeoStats::EVENT_DELBAN, "event_delban");

sub event_moduleload {
	my ($param1) = @_;
	NeoStats::ChanAlert($bot, "New Module Loaded: $param1");
}

sub event_moduleunload {
	my ($param1) = @_;
	NeoStats::ChanAlert($bot, "Module Unloaded: $param1");
}

sub event_server {
	my ($test) = @_;
	NeoStats::ChanAlert($bot, "New Server $test");
	my $server = NeoStats::FindServer($test);
	NeoStats::ChanAlert($bot, "Server Uplink $test->{uplink}");
}

sub event_squit {
	my ($server, $msg) = @_;
	NeoStats::ChanAlert($bot, "Server $server Squit: $msg");
}

sub event_ping {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "Ping $source");
}

sub event_pong {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "Pong $source");
	NeoStats::DBAStore("hello", "data", "This is a string data");
	my $text = NeoStats::DBAFetch("hello", "data");
	NeoStats::debug($text);
	Dump(NeoStats::DBAFetchRows("hello"));
	NeoStats::DBADelete("hello", "data");

#	my $testvar = new NeoStats::NV("users");
#	Dump($testvar->{fish});
#	NeoStats::debug($testvar->{fish}->{nick});
#	while ( my ($key, $value) = each(%$testvar)) {
#		NeoStats::debug("User Key => $value");
#	    	while ( my ($key1, $value1) = each(%$key)) {
#        		NeoStats::debug("$key: $key1 => $value1");
#    		}
#	}
#	NeoStats::debug "Found Fish" if exists $testvar->{fish};
#	my $hashtest = $testvar->{fish};
#	while ( my ($key, $value) = each(%$hashtest)) {
#		NeoStats::debug("$key => $value");
#	}
#
#	my $servers = new NeoStats::NV("Servers");
#	while ( my ($key, $value) = each(%$servers)) {
#		NeoStats::debug("Hostserv Key => $value, $key");
#	    	while ( my ($key1, $value1) = each(%$value)) {
#        		NeoStats::debug("HS: $key: $key1 => $value1");
#    		}
#	}

	my $hostserv = new NeoStats::NV("HostServ");
#	my $hashtest = $hostserv->{1};
	while ( my ($key, $value) = each(%$hostserv)) {
		NeoStats::debug("Hostserv Key => $value, $key");
	    	while ( my ($key1, $value1) = each(%$value)) {
        		NeoStats::debug("HS: $key: $key1 => $value1");
    		}
	}
#	Dump($hostserv);
#	NeoStats::debug("Debug HostServ: ".$hostserv." Ok");
#	Dump($hostserv);
#	NeoStats::debug("Var: HostServ".$hostserv->{0}."Fin");
#	$hashtest->{nick} = "ghaha";
#	$hostserv->{0}->{nick} = "haha";
#	NeoStats::debug("HS Nick: ".$hashtest->{nick});
	my $blah;
	$blah->{nick} = "Gheheh";
	$blah->{host} = "w00p.com";
	$blah->{vhost} = "goog.com";
	$blah->{passwd} = "pass";
	$blah->{added} = "Fish";
	$blah->{tslastused} = "0";
	NeoStats::DBASaveData("blah", $blah->{nick}, $blah);
	my $ha = NeoStats::DBAFetchAllData("blah");
	Dump($ha);
	NeoStats::debug("ha.".$ha->{'Gheheh'}->{passwd});
#	Dump($blah);
#	Dump($hostserv);
	$hostserv->AddNode(-2, $blah);
	while ( my ($key, $value) = each(%$hostserv)) {
		NeoStats::debug("Hostserv Key => $value, $key");
	    	while ( my ($key1, $value1) = each(%$value)) {
        		NeoStats::debug("HS: $key: $key1 => $value1");
    		}
	}
	$blah->{host} = "Woop.net";
	$hostserv->ModNode(0, $blah);
	$hostserv->DeleteNode(4);
}

sub event_signon {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "Signon $source");
	my $user = NeoStats::FindUser($source);
	NeoStats::ChanAlert($bot, "Host: $user->{hostname}");
	NeoStats::ChanAlert($bot, "Server $user->{server}");
}

sub event_quit {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "Quit $source: $msg");
}

sub event_nickip {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "NickIP $source");
}

sub event_kill {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "KILL $target by $source: $msg");
}

sub event_globalkill {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "GLOBALKILL $target by $source: $msg");
}

sub event_localkill {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "LOCALKILL $target by $source: $msg");
}

sub event_serverkill {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "SEVERKILL $target by $source: $msg");
}

sub event_botkill {
	my ($target, $msg) = @_;
	NeoStats::ChanAlert($bot, "BOTKILL $target: $msg");
}

sub event_nick {
	my ($source, $target) = @_;
	NeoStats::ChanAlert($bot, "NICKChange $source: $target");
}

sub event_away {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "AwayChange $source");
}

sub event_umode {
	my ($source, $mode) = @_;
	NeoStats::ChanAlert($bot, "UMODE $source, $mode");
}

sub event_smode {
	my ($source, $mode) = @_;
	NeoStats::ChanAlert($bot, "SMODE $source, $mode");
}

sub event_newchan {
	my ($channel) = @_;
	NeoStats::ChanAlert($bot, "NewChan $channel");
}

sub event_delchan {
	my ($channel) = @_;
	NeoStats::ChanAlert($bot, "DelChan $channel");
}

sub event_join {
	my ($channel, $source) = @_;
	NeoStats::ChanAlert($bot, "Join $channel: $source");
	my $chan = NeoStats::FindChan($channel);
	NeoStats::ChanAlert($bot, "Channel users $chan->{users}");
}

sub event_part {
	my ($channel, $source, $msg) = @_;
	NeoStats::ChanAlert($bot, "Part $channel: $source: $msg");
}

sub event_partbot {
	my ($channel, $source, $msg) = @_;
	NeoStats::ChanAlert($bot, "Partbot $channel: $source: $msg");
}

sub event_emptychan {
	my ($channel, $source, $bot, $msg) = @_;
	NeoStats::ChanAlert($bot, "Empty $channel $source $bot: $msg");
}

sub event_kick {
	my ($channel, $source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "Kick $channel $source $target: $msg");
}

sub event_kickbot {
	my ($channel, $source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "Kickbot $channel $source $target: $msg");
}

sub event_topic {
	my ($channel, $source) = @_;
	NeoStats::ChanAlert($bot, "Topic $channel $source");
}

sub event_cmode {
	NeoStats::ChanAlert($bot, "ToDO");
}

sub event_private {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "Privmsg $source $target: $msg");
}

sub event_notice {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "NOTICE $source $target: $msg");
}

sub event_cprivate {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "CPRIVATE $source $target: $msg");
}

sub event_cnotice {
	my ($source, $target, $msg) = @_;
	NeoStats::ChanAlert($bot, "CNOTICE $source $target: $msg");
}

sub event_globops {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "GLOBOPS $source $msg");
}

sub event_chatops {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "CHATOPS $source: $msg");
}

sub event_wallops {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "WALLOPS $source: $msg");
}

sub event_ctcpversionrpl {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "CTCPVERSIONRPL $source: $msg");
}

sub event_ctcpversionreq {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "CTCPVERSIONREQ $source");
}

sub event_ctcpfingerrpl {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "CTCPFINGERRPL $source: $msg");
}

sub event_ctcpfingerreq {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "CTCPFINGERREQ $source");
}

sub event_ctcpactionreq {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "CTCPACTIONREQ $source");
}

sub event_ctcptimerpl {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "CTCPTIMERPL $source: $msg");
}

sub event_ctcptimereq {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "CTCPTIMEREQ $source");
}

sub event_ctcppingrpl {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "CTCPPINGRPL $source: $msg");
}

sub event_ctcppingreq {
	my ($source) = @_;
	NeoStats::ChanAlert($bot, "CTCPPINGREQ $source");
}

sub event_dccsend {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "DCCSEND $source: $msg");
}

sub event_dccchat {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "DCCCHAT $source: $msg");
}

sub event_dccmsg {
	my ($source, $msg) = @_;
	NeoStats::ChanAlert($bot, "DCCMSG $source: $msg");
}

sub event_addban {
	NeoStats::ChanAlert($bot, "AddBan");
}

sub event_delban {
	NeoStats::ChanAlert($bot, "DelBan");
}



sub setupbot {
	my $botinfo;
	NeoStats::debug("Setup");
	$botinfo->{nick} = "Fishy";
	$botinfo->{altnick} = "Fishy2";
	$botinfo->{ident} = "fish";
	$botinfo->{host} = "Host.com";
	$botinfo->{gecos} = "My Gecos";
	$bot = NeoStats::AddBot($botinfo, NeoStats::BOT_FLAG_ROOT);
	$botinfo->{nick} = "fishy2";
	NeoStats::AddBot($botinfo, NeoStats::BOT_FLAG_ROOT);
	NeoStats::debug("Added Second Bot $botinfo->{nick}");
	NeoStats::DelBot($botinfo->{nick});
	#add a command
	my @helptext = ('Test Perl Module Interface', 'This Command Tests the Perl Module Interface', 'By Executing several of the Perl API commands against you (but it wont kill you!)');
	my $cmd = {
		cmd => 'TESTAPI',
		minparams => '0',
		ulevel => '0',
		flags => '0',
	};
	$cmd->{helptext} = \@helptext;
	NeoStats::AddCmd($bot, $cmd, 'cmd_cb_test');
	$cmd = {
		cmd => 'test2',
		minparams => '1',
		ulevel => '0',
		flags => '0',
	};
	$cmd->{helptext} = \@helptext;
	NeoStats::AddCmd("SecureServ", $cmd, 'cmd_cb_test');
	NeoStats::debug(NeoStats::DelCmd($bot, $cmd->{cmd}));
	NeoStats::ChanAlert($bot, "Loaded up and ready to rock and role");

	NeoStats::AddTimer(NeoStats::TIMER_TYPE_INTERVAL, "Test Timer", 10, "timer_cb");

}

sub timer_cb {
	NeoStats::ChanAlert($bot, "Timer Alert");
	return NeoStats::NS_SUCCESS;
}

sub cmd_cb_test {
	my ($cmd, $who, $params) = @_;
	NeoStats::debug("Got $cmd Command from $who: $params ");
	NeoStats::PrefMsg($bot, $who, "You Sent me this: $params");

	# Now Exercise the perl Module API 
	NeoStats::ChanAlert($bot, "PrivMsg: ".NeoStats::PrivMsg($bot, $who, "Testing PrivMsg"));
	NeoStats::ChanAlert($bot, "Notice: ".NeoStats::Notice($bot, $who, "Testing Notice"));
	NeoStats::ChanAlert($bot, "ChanPrivMsg: ".NeoStats::ChanPrivMsg($bot, "#services", "Testing ChanPrivMsg"));
	NeoStats::ChanAlert($bot, "ChanNotice: ".NeoStats::ChanNotice($bot, "#services", "Testing ChanNotice"));
	NeoStats::ChanAlert($bot, "Globops: ".NeoStats::Globops($bot, "Testing Globops"));
	NeoStats::ChanAlert($bot, "Wallops: ".NeoStats::Wallops($bot, "Testing Wallops"));
	NeoStats::ChanAlert($bot, "Numeric: ".NeoStats::Numeric(123, $who, "Testing Numeric"));
	#XXX Todo to define UMODES
	NeoStats::ChanAlert($bot, "Umode: ".NeoStats::Umode($bot, $bot, 0));
	NeoStats::ChanAlert($bot, "Join: ".NeoStats::Join($bot, "#testing", "+t"));
	NeoStats::ChanAlert($bot, "Invite: ".NeoStats::Invite($bot, "#testing", $who));
	NeoStats::ChanAlert($bot, "SvsJoin: ".NeoStats::SvsJoin($bot, $who, "#testing"));
	NeoStats::ChanAlert($bot, "CMode: ".NeoStats::SvsJoin($bot, "#testing", "+nl", "10"));
	NeoStats::ChanAlert($bot, "ChanUserMode: ".NeoStats::ChanUserMode($bot, "#testing", "+o", $who));
	NeoStats::ChanAlert($bot, "Topic: ".NeoStats::Topic($bot, "#testing", "My Lovely Topic"));
	NeoStats::ChanAlert($bot, "Kick: ".NeoStats::Kick($bot, "#testing", $who, "Cause I like Kicks"));
	NeoStats::ChanAlert($bot, "Part: ".NeoStats::Part($bot, "#testing", "Its a Boring Channel"));
	NeoStats::ChanAlert($bot, "NickChange: ".NeoStats::NickChange($bot, "NewNick"));
	NeoStats::ChanAlert($bot, "NickChange: ".NeoStats::NickChange("NewNick", $bot));
	NeoStats::ChanAlert($bot, "Finished. All Commands Should have returned 1");		


	return NeoStats::NS_SUCCESS;
}

sub shutdownbot {
	NeoStats::debug("Shutdown");
}

