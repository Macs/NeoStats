BEGIN {
  $INC{'NeoStats.pm'} = 'DUMMY';
}

use File::Spec();
use File::Basename();
use Symbol();
my ($haveStorable) = eval 'require Storable;';
Storable->import(qw/nfreeze thaw/) if $haveStorable;
use MIME::Base64;

{
  package NeoStats;
  use base qw(Exporter);
  use strict;
  use warnings;
  our %EXPORT_TAGS = ( all => [
			       qw(register hook_server hook_command),
			       qw(hook_print hook_timer unhook print command),
			       qw(find_context get_context set_context),
			       qw(get_info get_prefs emit_print nickcmp),
			       qw(get_list context_info strip_code),
			       qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD),
			       qw(PRI_LOWEST EAT_NONE EAT_NeoStats NS_FAILURE),
			       qw(NS_SUCCESS KEEP REMOVE),
			      ],
		       constants => [
				     qw(EVENT_MODULELOAD EVENT_MODULEUNLOAD PRI_NORM PRI_LOW),
				     qw(PRI_LOWEST EAT_NONE EAT_NeoStats),
				     qw(NS_FAILURE NS_SUCCESS FD_READ FD_WRITE),
				     qw(FD_EXCEPTION FD_NOTSOCKET KEEP REMOVE),
				    ],
		       hooks => [
				 qw(hook_server hook_command),
				 qw(hook_print hook_timer unhook),
				],
		       util => [
				qw(register print command find_context),
				qw(get_context set_context get_info get_prefs),
				qw(emit_print nickcmp get_list context_info),
				qw(strip_code),
			       ],
		     );

  our @EXPORT = @{$EXPORT_TAGS{constants}};
  our @EXPORT_OK = @{$EXPORT_TAGS{all}};

  sub register {
    if (@_ != 5) {
      NeoStats::debug("Invalid Number of arguments to register");
      return NeoStats::NS_FAILURE;
    }
    my ($package) = caller;
    my $pkg_info = NeoStats::Embed::pkg_info( $package );
    my $filename = $pkg_info->{filename};
	
	if ($pkg_info->{type} != 0) {
		NeoStats::debug("Extension tried to register as a module");
		return NeoStats::NS_FAILURE;
	}

    my ($name, $version, $description, $startupcb, $shutdowncb) = @_;
    $description = "" unless defined $description;


    $pkg_info->{name} = $name;
    $pkg_info->{version} = $version;
    $pkg_info->{description} = $description;
    $pkg_info->{gui_entry} =
      NeoStats::Internal::register( $pkg_info->{name}, $pkg_info->{version}, $pkg_info->{description});
    $startupcb = NeoStats::Embed::fix_callback( $package, $startupcb );
    $shutdowncb = NeoStats::Embed::fix_callback( $package, $shutdowncb );
    $pkg_info->{shutdown} = $shutdowncb;
    $pkg_info->{startup} = $startupcb;

    # keep with old behavior
    return NeoStats::NS_SUCCESS;
  }

  sub registerextension {
    if (@_ != 4) {
      NeoStats::debug("Invalid Number of arguments to registerextension");
      return NeoStats::NS_FAILURE;
    }
    my ($package) = caller;
    my $pkg_info = NeoStats::Embed::pkg_info( $package );
    my $filename = $pkg_info->{filename};
	
	if ($pkg_info->{type} != 1) {
		NeoStats::debug("Perl Module tried to register as a extension");
		return NeoStats::NS_FAILURE;
	}

    my ($name, $version, $startupcb, $shutdowncb) = @_;
    $pkg_info->{name} = $name;
    $pkg_info->{version} = $version;
    $pkg_info->{gui_entry} =
      NeoStats::Internal::registerextension( $pkg_info->{name}, $pkg_info->{version});
    $startupcb = NeoStats::Embed::fix_callback( $package, $startupcb );
    $shutdowncb = NeoStats::Embed::fix_callback( $package, $shutdowncb );
    $pkg_info->{shutdown} = $shutdowncb;
    $pkg_info->{startup} = $startupcb;

    # keep with old behavior
    return NeoStats::NS_SUCCESS;
  }


  sub hook_event {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to hook_event");
      return NeoStats::NS_FAILURE;
    }

    my $event = shift;
    my $callback = shift;
    my $options = shift;
    my ($package) = caller;
    my $flags = 0;
    my $data = "";
    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if ( ref( $options ) eq 'HASH' ) {
      if ( exists( $options->{flags} ) && defined( $options->{flags} ) ) {
        $flags = $options->{flags};
      }
      if ( exists( $options->{data} ) && defined( $options->{data} ) ) {
        $data = $options->{data};
      }
    }
    
    my $pkg_info = NeoStats::Embed::pkg_info( $package );
    my $hook =  NeoStats::Internal::hook_event( $event, $flags, $callback, $data);
    if ( defined ( $hook )) {
      push @{$pkg_info->{hooks}}, $event;
      return NeoStats::NS_SUCCESS;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }
  
  sub unhook_event {
    NeoStats::debug("todo");
    return NeoStats::NS_FAILURE;
  }


# AddBot(botinfo, botflag)
  sub AddBot {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to AddBot");
      return NeoStats::NS_FAILURE;
    }

    my $botinfo = shift;
    my $botflag = shift;
    my $data = shift;
    my ($package) = caller;
#    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if (!ref( $botinfo ) eq 'HASH' ) {
      return NeoStats::NS_FAILURE;
    }
    if ((!exists( $botinfo->{nick} )) || (!defined( $botinfo->{nick} ))) {
      NeoStats::debug("Botinfo->{nick} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{altnick} )) || (!defined( $botinfo->{altnick} ))) {
      NeoStats::debug("Botinfo->{altnick} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{ident} )) || (!defined( $botinfo->{ident} ))) {
      NeoStats::debug("Botinfo->{ident} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{host} )) || (!defined( $botinfo->{host} ))) {
      NeoStats::debug("Botinfo->{host} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botinfo->{gecos} )) || (!defined( $botinfo->{gecos} ))) {
      NeoStats::debug("Botinfo->{gecos} not defined");
      return NeoStats::NS_FAILURE;
    }    
    
    my $bot =  NeoStats::Internal::AddBot( $botinfo, $botflag, $data);
    if ( defined ( $bot )) {
      return $bot;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }
  sub DelBot {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of arguments to DelBot");
      return NeoStats::NS_FAILURE;
    }
    my $botname = shift;
    my $reason = shift;
    if (!defined($reason)) {
      $reason = "Unknown";
    }
    return NeoStats::Internal::DelBot($botname, $reason);
  }    

  sub FindUser {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of arguments to FindUser");
      return NeoStats::NS_FAILURE;
    }
    my $nick = shift;
    return NeoStats::Internal::FindUser($nick);
  }

  sub FindServer {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of arguments to FindServer");
      return NeoStats::NS_FAILURE;
    }
    my $name = shift;
    return NeoStats::Internal::FindServer($name);
  }

  sub FindChan {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of arguments to FindChannel");
      return NeoStats::NS_FAILURE;
    }
    my $name = shift;
    return NeoStats::Internal::FindChannel($name);
  }
  
  sub AddCmd {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to AddCmd");
      return NeoStats::NS_FAILURE;
    }

    my $bot = shift;
    my $botcmd = shift;
    my $callback = shift;
    my $data = shift;
    my ($package) = caller;
    $callback = NeoStats::Embed::fix_callback( $package, $callback );
  
    if (!ref( $botcmd ) eq 'HASH' ) {
      NeoStats::debug("Botcmd is not a hash");
      return NeoStats::NS_FAILURE;
    }
    if ((!exists( $botcmd->{cmd} )) || (!defined( $botcmd->{cmd} ))) {
      NeoStats::debug("Botinfo->{cmd} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botcmd->{minparams} )) || (!defined( $botcmd->{minparams} ))) {
      NeoStats::debug("Botinfo->{minparams} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botcmd->{ulevel} )) || (!defined( $botcmd->{ulevel} ))) {
      NeoStats::debug("Botinfo->{ulevel} not defined");
      return NeoStats::NS_FAILURE;
    }    
    if ((!exists( $botcmd->{helptext} )) || (!defined( $botcmd->{helptext} ))) {
      NeoStats::debug("Botinfo->{helptext} not defined");
      return NeoStats::NS_FAILURE;
    }    
    #XXX Need to check size of the helptext array to make sure at least 2 lines 
    if ((!exists( $botcmd->{flags} )) || (!defined( $botcmd->{flags} ))) {
      NeoStats::debug("Botinfo->{flags} not defined");
      return NeoStats::NS_FAILURE;
    }    
    my $ret =  NeoStats::Internal::AddCommand( $bot, $botcmd, $callback);
    return $ret;
  }

  sub DelCmd {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to DelCmd");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $botcmd = shift;
    return NeoStats::Internal::DelCommand($bot, $botcmd);
  }

  sub PrefMsg {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to PreMsg");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $message = shift;
    return NeoStats::Internal::Prefmsg($bot, $target, $message);
  }

  sub ChanAlert {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to ChanAlert");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $message = shift;
    return NeoStats::Internal::ChanAlert($bot, $message);
  }

  sub PrivMsg {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to PrivMsg");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $to = shift;
    my $message = shift;
    return NeoStats::Internal::PrivMsg($bot, $to, $message);
  }

  sub Notice {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Notice");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $to = shift;
    my $message = shift;
    return NeoStats::Internal::Notice($bot, $to, $message);
  }

  sub ChanPrivMsg {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to ChanPrivMsg");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $to = shift;
    my $message = shift;
    return NeoStats::Internal::ChanPrivMsg($bot, $to, $message);
  }

  sub ChanNotice {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to ChanNotice");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $to = shift;
    my $message = shift;
    return NeoStats::Internal::ChanNotice($bot, $to, $message);
  }

  sub Globops {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to Globops");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $message = shift;
    return NeoStats::Internal::Globops($bot, $message);
  }

  sub Wallops {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to Wallops");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $message = shift;
    return NeoStats::Internal::Globops($bot, $message);
  }

  sub Numeric {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Numeric");
      return NeoStats::NS_FAILURE;
    }
    my $numeric = shift;
    my $target = shift;
    my $message = shift;
    return NeoStats::Internal::Numeric($numeric, $target, $message);
  }

  sub Umode {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Umode");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $umode = shift;
    return NeoStats::Internal::Umode($bot, $target, $umode);
  }

  sub Join {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Cmode");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $channel = shift;
    my $cmode = shift;
    return NeoStats::Internal::Join($bot, $channel, $cmode);
  }

  sub Part {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to Part");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $channel = shift;
    my $message = shift;
    return NeoStats::Internal::Part($bot, $channel, $message);
  }

  sub NickChange {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to NickChange");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $newnick = shift;
    return NeoStats::Internal::NickChange($bot, $newnick);
  }

  sub CMode {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to CMode");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $chan = shift;
    my $modes = shift;
    my $args = shift;
    return NeoStats::Internal::CMode($bot, $chan, $modes, $args);
  }

  sub ChanUserMode {
    if (@_ < 4) {
      NeoStats::debug("Invalid Number of arguments to ChanUserMode");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $chan = shift;
    my $modes = shift;
    my $target = shift;
    return NeoStats::Internal::ChanUserMode($bot, $chan, $modes, $target);
  }

  sub Kill {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Kill");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $reason = shift;
    return NeoStats::Internal::Kill($bot, $target, $reason);
  }

  sub Kick {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Kick");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $chan = shift;
    my $target = shift;
    my $reason = shift;
    return NeoStats::Internal::Kick($bot, $chan, $target, $reason);
  }

  sub Invite {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Invite");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $chan = shift;
    my $target = shift;
    return NeoStats::Internal::Invite($bot, $chan, $target);
  }

  sub Topic {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Topic");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $chan = shift;
    my $topic = shift;
    return NeoStats::Internal::Topic($bot, $chan, $topic);
  }

  sub SvsKill {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsKill");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $reason = shift;
    return NeoStats::Internal::SvsKill($bot, $target, $reason);
  }

  sub SvsMode {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsMode");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $mode = shift;
    return NeoStats::Internal::SvsKill($bot, $target, $mode);
  }

  sub SvsHost {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsHost");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $host = shift;
    return NeoStats::Internal::SvsHost($bot, $target, $host);
  }

  sub SvsJoin {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsJoin");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $chan = shift;
    return NeoStats::Internal::SvsJoin($bot, $target, $chan);
  }

  sub SvsPart {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsPart");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $chan = shift;
    return NeoStats::Internal::SvsPart($bot, $target, $chan);
  }

  sub Swhois {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguments to Swhois");
      return NeoStats::NS_FAILURE;
    }
    my $target = shift;
    my $swhois = shift;
    return NeoStats::Internal::Swhois($target, $swhois);
  }

  sub SvsNick {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SvsNick");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $target = shift;
    my $newnick = shift;
    return NeoStats::Internal::SvsNick($bot, $target, $newnick);
  }

  sub SMO {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to SMO");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $umodetarget = shift;
    my $message = shift;
    return NeoStats::Internal::SMO($bot, $umodetarget, $message);
  }

  sub Akill {
    if (@_ < 5) {
      NeoStats::debug("Invalid Number of arguments to Akill");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $host = shift;
    my $ident = shift;
    my $length = shift;
    my $message = shift;
    return NeoStats::Internal::Akill($bot, $host, $ident, $length, $message);
  }

  sub Rakill {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguments to Rakill");
      return NeoStats::NS_FAILURE;
    }
    my $bot = shift;
    my $host = shift;
    my $ident = shift;
    return NeoStats::Internal::Rakill($bot, $host, $ident);
  }

  sub AddTimer {
    if (@_ < 4) {
      NeoStats::debug("Invalid Number of arguments to AddTimer");
      return NeoStats::NS_FAILURE;
    }
    my $type = shift;
    my $name = shift;
    my $interval = shift;
    my $callback = shift;
    my ($package) = caller;
    $callback = NeoStats::Embed::fix_callback( $package, $callback );
    return NeoStats::Internal::AddTimer($type, $name, $interval, $callback);
  }

  sub DelTimer {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of arguments to DelTimer");
      return NeoStats::NS_FAILURE;
    }
    my $name = shift;
    return NeoStats::Internal::DelTimer($name);
  }
 
  sub DBAStore {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of arguements to DBAStore");
      return NeoStats::NS_FAILURE;
    }
    my $table = shift;
    my $name = shift;
    my $data = shift;
    if ((ref($data) eq 'ARRAY') || (ref($data) eq 'HASH')) {
      NeoStats::debug("Data must be a string in DBAStore");
      return NeoStats::NS_FAILURE;
    }
    return NeoStats::Internal::DBAStore($table, $name, $data);
  }
  sub DBAFetch {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguements to DBAFetch");
      return;
    }
    my $table = shift;
    my $name = shift;
    return NeoStats::Internal::DBAFetch($table, $name);
  } 
  
  sub DBADelete {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of arguements to DBADelete");
      return NeoStats::NS_FAILURE;
    }
    my $table = shift;
    my $name = shift;
    return NeoStats::Internal::DBADelete($table, $name);
  }
  sub DBAFetchRows {
    if (@_ < 1) {
      NeoStats::debug("Invaid Number of Arguements to DBAFetchRows");
      return;
    }
    my $table = shift;
    return NeoStats::Internal::DBAFetchRows($table);
  }
  sub DBASaveData {
    if (@_ < 3) {
      NeoStats::debug("Invalid Number of Arguements to DBASaveData");
      return NeoStats::NS_FAILURE;
    }
    my $table = shift;
    my $name = shift;
    my $data = shift;
    $data = Storable::nfreeze($data);
    return NeoStats::Internal::DBAStore($table, $name, MIME::Base64::encode($data));
  }
  sub DBAFetchData {
    if (@_ < 2) {
      NeoStats::debug("Invalid Number of Arguements to DBAFetchData");
      return NeoStats::NS_FAILURE;
    }
    my $table = shift;
    my $name = shift;
    return Storable::thaw(MIME::Base64::decode(NeoStats::Internal::DBAFetch($table, $name)));
  }
  sub DBAFetchAllData {
    if (@_ < 1) {
      NeoStats::debug("Invalid Number of Arguements to DBAFetchAllData");
      return NeoStats::NS_FAILURE;
    }
    my $table = shift;
    my $frozenhash = NeoStats::Internal::DBAFetchRows($table);
    while ( my ($key, $value) = each(%$frozenhash)) {
       $frozenhash->{$key} = Storable::thaw(MIME::Base64::decode($value));
    }
    return $frozenhash;
  }  
  sub debug {
    my $text = shift @_;
    return 1 unless $text;
    if ( ref( $text ) eq 'ARRAY' ) {
      if ( $, ) {
        $text = join $, , @$text;
      } else {
        $text = join "", @$text;
      }
    }
    NeoStats::Internal::debug( $text );
    return 1;
  }

  sub printf {
    my $format = shift;
    NeoStats::debug( sprintf( $format, @_ ) );
  }

  sub strip_code {
    my $pattern =
      qr/\cB| #Bold
       \cC\d{0,2}(?:,\d{0,2})?| #Color
       \cG| #Beep
       \cO| #Reset
       \cV| #Reverse
       \c_  #Underline
      /x;

    if ( defined wantarray ) {
      my $msg = shift;
      $msg =~ s/$pattern//g;
      return $msg;
    } else {
      $_[0] =~ s/$pattern//g;
    }
  }

}

$SIG{__WARN__} = sub {
  my $message = shift @_;
  my ($package) = caller;
  my $pkg_info = NeoStats::Embed::pkg_info( $package );
  
  if( $pkg_info ) {
    $message =~ s/\(eval \d+\)/$pkg_info->{filename}/;
  }
  NeoStats::debug( $message );
};

{
  package NeoStats::Embed;
  use strict;
  use warnings;

  # list of loaded scripts keyed by their package names
  our %scripts;
  sub expand_homedir {
    my $file = shift @_;
    
    if( $^O eq "MSWin32" ) {
      $file =~ s/^~/$ENV{USERPROFILE}/;
    } else {
      $file =~
        s{^~}{
          (getpwuid($>))[7] ||  $ENV{HOME} || $ENV{LOGDIR}
        }ex;
    }
    return $file;
  }
  sub file2pkg {
   
    my $string = File::Basename::basename( shift @_ );
    $string =~ s/\.pl$//i;
    $string =~ s|([^A-Za-z0-9/])|'_'.unpack("H*",$1)|eg;

    return "NeoStats::Module::" . $string;
  }

  sub pkg_info {
    my $package = shift @_;
    return $scripts{$package};
  }

  sub fix_callback {
    my ($package, $callback) = @_;

    unless( ref $callback ) {
      # change the package to the correct one in case it was hardcoded
      $callback =~ s/^.*:://;
      $callback = qq[${package}::$callback];
    }
    return $callback;
  }

  sub load {
    my $file = expand_homedir( shift @_ );

    my $package = file2pkg( $file );
# no need for this, as we only load one file per interpreter 
#    print $package;
#    if ( exists $scripts{$package} ) {
#      my $pkg_info = pkg_info( $package );
#      my $filename = File::Basename::basename( $pkg_info->{filename} );
#      NeoStats::debug( qq{'$filename' already loaded from '$pkg_info->{filename}'.\n} );
#      NeoStats::debug( 'If this is a different script then it rename and try loading it again.' );
#      return 2;
#    }

    if ( open FH, $file ) {
      my $source = do {local $/; <FH>};
      close FH;

      if ( my $replacements = $source =~ s/^\s*package ([\w:]+).*?;//mg ) {
        my $original_package = $1;

        if ( $replacements > 1 ) {
          NeoStats::debug( "Too many package defintions, only 1 is allowed\n" );
          return 1;
        }

        # fixes things up for code calling subs with fully qualified names
        $source =~ s/${original_package}:://g;

      }

      # this must come before the eval or the filename will not be found in
      # NeoStats::register
      $scripts{$package}{filename} = $file;
	  $scripts{$package}{type} = 0;

      {
#        no strict; no warnings;
        eval "package $package; $source;";
      }

      if ( $@ ) {
        # something went wrong
        NeoStats::debug( "Error loading '$file':\n$@\n" );

        # make sure the script list doesn't contain false information
        unload( $scripts{$package}{filename} );
        return 1;
      }

    } else {
      NeoStats::debug( "Error opening '$file': $!\n" );
      return 2;
    }
    
    return 0;
  }
 
  
    sub loadextension {
    my $file = expand_homedir( shift @_ );

    my $package = file2pkg( $file );

    if ( open FH, $file ) {
      my $source = do {local $/; <FH>};
      close FH;

      if ( my $replacements = $source =~ s/^\s*package ([\w:]+).*?;//mg ) {
        my $original_package = $1;

        if ( $replacements > 1 ) {
          NeoStats::debug( "Too many package defintions, only 1 is allowed\n" );
          return 1;
        }

        # fixes things up for code calling subs with fully qualified names
        $source =~ s/${original_package}:://g;

      }

      # this must come before the eval or the filename will not be found in
      # NeoStats::registerextension
      $scripts{$package}{filename} = $file;
	  $scripts{$package}{type} = 1;
	  
      {
#        no strict; no warnings;
        eval "package $package; $source;";
      }

      if ( $@ ) {
        # something went wrong
        NeoStats::debug( "Error loading extension '$file':\n$@\n" );

        # make sure the script list doesn't contain false information
        unload( $scripts{$package}{filename} );
        return 1;
      }

    } else {
      NeoStats::debug( "Error opening '$file': $!\n" );
      return 2;
    }
    
    return 0;
  }
  
  sub unload {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );

    if( $pkg_info ) {

      if( exists $pkg_info->{hooks} ) {
        for my $hook ( @{$pkg_info->{hooks}} ) {
          NeoStats::Internal::unhook_event( $hook );
        }
      }

    # take care of the shutdown callback
      if( exists $pkg_info->{shutdown} ) {
        if( ref $pkg_info->{shutdown} eq 'CODE' ) {
          $pkg_info->{shutdown}->();
        } elsif( $pkg_info->{shutdown} ) {
          eval {
            no strict 'refs';
            &{$pkg_info->{shutdown}};
          };
        }
      }
      
      Symbol::delete_package( $package );
      delete $scripts{$package};
      return NeoStats::NS_SUCCESS;
    } else {
      return NeoStats::NS_FAILURE;
    }
  }

  sub reload {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );
    my $fullpath = $file;

    if( $pkg_info ) {
      $fullpath = $pkg_info->{filename};
      unload( $file );
    }
    load( $fullpath );
    return NeoStats::NS_SUCCESS;
  }

  sub unload_all {
    for my $package ( keys %scripts ) {
      unload( $scripts{$package}->{filename} );
    }
    return NeoStats::NS_SUCCESS;
  }

  sub sync {
    my $file = shift @_;
    my $package = file2pkg( $file );
    my $pkg_info = pkg_info( $package );
    if( exists $pkg_info->{startup} ) {
        if( ref $pkg_info->{startup} eq 'CODE' ) {
          $pkg_info->{startup}->();
        } elsif( $pkg_info->{startup} ) {
          eval {
            no strict 'refs';
            &{$pkg_info->{startup}};
          };
        }
      }
  }    


}
