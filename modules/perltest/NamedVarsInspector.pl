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
my $bot;

NeoStats::register( "NVDebug", "1.0", "Named Vars Inspection Script", "setupnvdebug", "");

sub setupnvdebug {
	my $botinfo;
	$botinfo->{nick} = "NVDebug";
	$botinfo->{altnick} = "NVDebug1";
	$botinfo->{ident} = "NV";
	$botinfo->{host} = "debug.host";
	$botinfo->{gecos} = "NamedVar Debug Bot";
	$bot = NeoStats::AddBot($botinfo, NeoStats::BOT_FLAG_ROOT);
	#add a command
	my @helptext = ('List All Available NamedVars');
	my $cmd = {
		cmd => 'LIST',
		minparams => '0',
		ulevel => '200',
		flags => '0',
	};
	$cmd->{helptext} = \@helptext;
	NeoStats::AddCmd($bot, $cmd, 'cmd_cb_list');
	my @helptext2 = ('Inspect a Named Var Entry');
	$cmd = {
		cmd => 'INSPECT',
		minparams => '1',
		ulevel => '0',
		flags => '0',
	};
	$cmd->{helptext} = \@helptext;
	NeoStats::AddCmd($bot, $cmd, 'cmd_cb_inspect');
}

sub cmd_cb_list {
	my ($cmd, $who, $params) = @_;
	
	my $NVList = new NeoStats::NV("NamedVars");
	NeoStats::PrefMsg($bot, $who, "No of NamedVar Entries: ".keys(%$NVList));
	while ( my ($key, $value) = each(%$NVList)) {
		NeoStats::PrefMsg($bot, $who, "NamedVar Name: $key");
	}
	NeoStats::PrefMsg($bot, $who, "End of List.");
	return NeoStats::NS_SUCCESS;
}

sub cmd_cb_inspect {
	my ($cmd, $who, $param) = @_;
	my @args = split(/ /, $param);
	
	my $NVList = new NeoStats::NV($args[1]);
	if (!$NVList) {
		NeoStats::PrefMsg($bot, $who, "Can't find NamedVar $args[1]");
		return NeoStats::NS_SUCCESS;
	}
	NeoStats::PrefMsg($bot, $who, "No Of Entries: ".keys(%$NVList));
	while ( my ($key, $value) = each(%$NVList)) {
		NeoStats::PrefMsg($bot, $who, "NamedVar Name: $key");
		while ( my ($key2, $value2) = each (%$value)) {
			NeoStats::PrefMsg($bot, $who, "   Option: $key2 Value: $value2");
		}
	}
	NeoStats::PrefMsg($bot, $who, "End of List.");
	return NeoStats::NS_SUCCESS;
}


