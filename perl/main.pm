package main {
    use lib './lib';
    use constant clib_file => "spo_lab2";

    use strict;
    use warnings;

    my $buildstor_mod = "CalledError";
    eval "require $buildstor_mod";

    use String::Util qw(trim);

    use part1;
    use part2;

    my %command = (
        'help'    => sub {
            print("Режимы работы:\n");
            print("\tTASK_1 - Выводит список подключенных дисков\n");
            print("\tTASK_2 <disk> - Работа с xfs\n");
            print("\texit - Выйти\n");
            return 1;
        },
        'TASK_1'    => sub {
            part1::exec();
            return 1;
        },
        'TASK_2'    => sub {
            part2::exec($_[0]);
            return 1;
        },
        'exit'    => sub {
            return 0;
        },
        'default' => sub {
            print("Неизвестная команда: $_[0]\nДля просмотра всех команд введите help\n");
            return 1;
        },
        ''        => sub { return 1; }
    );

    sub mainFunc {
        my $fun = 0;
        my $comm;
        my @args;
        do {{
            print("Введите режим работы: ");
            my $input = <STDIN>;
            ($comm, @args) = split(m/\s+/, trim($input));
            $comm = "" unless (defined($comm));
            @args = ("$comm") unless (scalar @args > 0);
            if(exists($command{$comm})) {
                $fun = $command{$comm};
            }else {
                @args = ($comm);
                $fun = $command{'default'}
            }
        }} while ($fun->(@args));
    }
}

main::mainFunc();
