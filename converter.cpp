#include <iostream>
#include <anthy/anthy.h>
#include <getopt.h>

class exception : public std::exception {
public:
    exception(const char* msg_) throw() : msg(msg_) {};
    virtual const char *what() const throw() { return msg; };
private:
    const char* msg;
};

std::string convert(anthy_context_t ac, const std::string in)
{
    int err;
    std::string out_str;
    err = anthy_set_string(ac, in.c_str());
    if (err < 0)
        throw exception("anthy_set_string failed");
    anthy_conv_stat conv_stat;
    err = anthy_get_stat(ac, &conv_stat);
    if (err < 0)
        throw exception("anthy_get_stat failed");
    for (int i=0; i<conv_stat.nr_segment; i++) {
        char buf[256];
        int len;
        len = anthy_get_segment(ac, i, 0, buf, 256);
        if (len < 0)
            throw exception("anthy_get_segment failed");
        out_str += std::string(buf);
    }
    anthy_reset_context(ac);
    return out_str;
}

void convert_lines(anthy_context_t ac)
{
    std::string line;
    while (std::getline(std::cin, line)) {
        std::string out = convert(ac, line.c_str());
        std::cout << out << std::endl;
    }
}


int
main(int argc, char **argv)
{
    int err;
    int use_utf8 = 0;
    int option_index = 0;

    while (1) {
      static struct option long_options[] = {
	{"help", 0, 0, 'h'},
	{"utf8", 0, 0, 'u'},
	{0, 0, 0, 0}
      };
      char c = getopt_long(argc, argv, "hu", long_options, &option_index);
      if (c == -1)
	break;
      switch (c) {
      case 'h':
	std::cout << "Usage: anthy-converter [OPTION]" << std::endl;
	return 0;
      case 'u':
	use_utf8 = 1;
	break;
      }
    }
    anthy_context_t ac;
    err = anthy_init();
    if (err) {
        std::cerr << "failed to init anthy" << std::endl;
        return -1;
    }
    err = anthy_set_personality("");
    ac = anthy_create_context();
    err = anthy_context_set_encoding(ac, use_utf8 ? ANTHY_UTF8_ENCODING : ANTHY_EUC_JP_ENCODING);
#ifdef HAS_ANTHY_SET_RECONVERSION_MODE
    err = anthy_set_reconversion_mode(ac, ANTHY_RECONVERT_DISABLE);
#endif
    convert_lines(ac);
    anthy_release_context(ac);
    anthy_quit();
    return 0;
}
