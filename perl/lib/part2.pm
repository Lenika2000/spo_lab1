package part2;

use strict;
use warnings FATAL => 'all';

use FFI::Platypus 1.00;
use FFI::Platypus::API;
use FFI::CheckLib 0.06;
use String::Util qw(trim);
use Switch;

use constant SUCCESS => 0;
use constant UNSUPPORTED_FILESYSTEM => 1;
use constant CANT_READ_SB => 2;
use constant CANT_READ_DINODE => 3;

my $ffi = FFI::Platypus->new(api => 1);

{
    $ffi->lib(FFI::CheckLib::find_lib_or_die lib => main::clib_file);

    $ffi->attach("init_xfs" => [ "string" ] => "int");
    $ffi->attach("xfs_ls" => [ "void" ] => "string");
    $ffi->attach("xfs_pwd" => [ "void" ] => "string");
    $ffi->attach("xfs_cd_perl" => [ "string" ] => "string");
}

my $currDirName = '/';

my %command = (
    'default' => sub {
        print("Неизвестная команда: $_[0]\nДля просмотра всех возможных команд введите help\n");
        return 1;
    },
    ''        => sub {return 1;},
    'exit'    => sub {return 0;},
    'help'    => sub {
        print("Help:\n");
        print("\tcp <file> <host_file> \t Скопировать файлы или директории\n");
        print("\tpwd \t Показать текущую директорию\n");
        print("\tls \t Вывести файлы с их атрибутами в директории\n");
        print("\tcd <dir> \t Переход в другую директорию\n");
        print("\thelp \t Получить список доступных команд\n");
        print("\texit \t Выход\n");
        return 1;
    },
    'ls'      => sub {
        my $ls = xfs_ls();
        print($ls);
        return 1;
    },
    'cp'      => sub {
        copyToHost($_[0], $_[1]);
        return 1;
    },
    'cd'      => sub {
        my $cd =  xfs_cd_perl($_[0]);
        print($cd);
        return 1;
    },
);


sub catch_init_error {
    my $ret = $_[0];
    unless ($ret == 0) {
        switch($ret) {
            case UNSUPPORTED_FILESYSTEM {die("Данная файловая система не поддерживается. Попробуйте еще раз.\n");}
                case CANT_READ_SB {die("Невозможно прочитать суперблок. Проверьте корректность введенного пути к xfs.\n");}
                    case CANT_READ_DINODE {die("Невозможно прочитать содержимое dinode\n");}
                        default {die("Неизвестная ошибка\n");}
        }
    }
}

sub interact {
    my $fun = 0;
    my $comm;
    my @args;
    do {{
        $currDirName = xfs_pwd();
        print("$currDirName > ");
        my $input = <STDIN>;
        ($comm, @args) = split(m/\s+/, trim($input));
        $comm = "" unless (defined($comm));
        @args = ("$comm") unless (scalar @args > 0);
        if (exists($command{$comm})) {$fun = $command{$comm};}
        else {
            @args = ($comm);
            $fun = $command{'default'}
        }
    }} while ($fun->(@args));
}

sub exec {
    print("===============================\n");
    print("Начало работы с XFS!\n");
    my $disk_name = $_[0];
    catch_init_error(init_xfs($disk_name));
    interact();
    # part_2_exit();
    print("===============================\n");
}

