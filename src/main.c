// TODO: replace matchlist with a hashmap in parsegonf
// TODO: better README
// TODO: manpage?
// TODO: maybe do configurable prefix?
// TODO: clean up throwing errors in parse.c

int process_args(int argc, char **argv);
int main(int argc, char **argv){
    return process_args(argc, argv);
}
