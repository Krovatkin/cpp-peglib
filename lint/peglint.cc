//
//  peglint.cc
//
//  Copyright (c) 2015 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <peglib.h>
#include <fstream>

using namespace std;

bool read_file(const char* path, vector<char>& buff)
{
    ifstream ifs(path, ios::in | ios::binary);
    if (ifs.fail()) {
        return false;
    }

    buff.resize(static_cast<unsigned int>(ifs.seekg(0, ios::end).tellg()));
    if (!buff.empty()) {
        ifs.seekg(0, ios::beg).read(&buff[0], static_cast<streamsize>(buff.size()));
    }
    return true;
}

int main(int argc, const char** argv)
{
    auto opt_ast = false;
    auto opt_help = false;
    vector<const char*> path_list;

    int argi = 1;
    while (argi < argc) {
        auto arg = argv[argi++];
        if (string("--help") == arg) {
            opt_help = true;
        } else if (string("--ast") == arg) {
            opt_ast = true;
        } else {
            path_list.push_back(arg);
        }
    }

    if (path_list.empty() || opt_help) {
        cerr << "usage: peglint [--ast] [grammar file path] [source file path]" << endl;
        return 1;
    }

    // Check PEG grammar
    auto syntax_path = path_list[0];

    vector<char> syntax;
    if (!read_file(syntax_path, syntax)) {
        cerr << "can't open the grammar file." << endl;
        return -1;
    }

    peglib::peg peg;

    peg.log = [&](size_t ln, size_t col, const string& msg) {
        cerr << syntax_path << ":" << ln << ":" << col << ": " << msg << endl;
    };

    if (!peg.load_grammar(syntax.data(), syntax.size())) {
        return -1;
    }

    if (path_list.size() < 2) {
        return 0;
    }

    // Check source
    auto source_path = path_list[1];

    vector<char> source;
    if (!read_file(source_path, source)) {
       auto beg = source_path;
       auto end = source_path + strlen(source_path);
	    source.assign(beg, end);
       source_path = "[commendline]";
    }

    peg.log = [&](size_t ln, size_t col, const string& msg) {
        cerr << source_path << ":" << ln << ":" << col << ": " << msg << endl;
    };

    if (opt_ast) {
	    peg.enable_ast();

	    std::shared_ptr<peglib::Ast> ast;
	    if (!peg.parse_n(source.data(), source.size(), ast)) {
	        return -1;
	    }

	    peglib::AstPrint().print(*ast);
    } else {
	    if (!peg.parse_n(source.data(), source.size())) {
	        return -1;
	    }
    }

    return 0;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
