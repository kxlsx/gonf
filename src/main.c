// TODO: test cli
// TODO: maybe do configurable prefix?
// TODO: error test flag parser
// TODO: error test flag parser parser
// TODO: pass FILES ** to lexer and parser?
// TODO: DOCUMENT SHIT
// TODO: handle SIGINT and stuff
// TODO: rewrite comp.c

int process_args(int argc, char **argv);
int main(int argc, char **argv){
    return process_args(argc, argv);
}
