package part1;
use lib '.';

use strict;
use warnings FATAL => 'all';

use FFI::Platypus 1.00;
use FFI::Platypus::API;
use FFI::CheckLib 0.06;

{
    my $ffi = FFI::Platypus->new(api => 1);
    $ffi->lib(FFI::CheckLib::find_lib_or_die lib =>  main::clib_file);
    $ffi->attach('run_list_mode' => [ 'void' ] => 'string');
}

sub exec {
    my $list_result = run_list_mode();
    print($list_result);
}

