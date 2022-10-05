// TODO: replace matchlist with a hashmap in parsegonf
// TODO: replace strdup in parsegonf with something better (mempool?) 
// TODO: better README
// TODO: maybe do configurable prefix?
// TODO: manpage?

int process_args(int argc, char **argv);
int main(int argc, char **argv){
    return process_args(argc, argv);
}
