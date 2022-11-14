// isa.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

#include <string>
#include <string.h>

#include <list>
#include <iterator>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXHOSTNAMELEN 128 // idk if there's a defined limit to this, ping.c has 128
#define PORT_HTTP 80
#define PORT_HTTPS 443

using namespace std;

void print_slist(list<string> lst, bool showlines = false, string delimit = "") {
    list<string>::iterator i;
    int l = 0;
    for (i = lst.begin(); i != lst.end(); ++i) {
        if (showlines) {
            cout << l << "  ";
            l++;
        }
        cout <<  delimit << *i << delimit << "\n";
    }
}

string delete_char(string s, char c){
    char buf[s.length() + 1] = {0};
    int j = 0;
    for (int i = 0; i < s.length(); i++){
        if (s[i] != c){
            buf[j++] = s[i];
        }
    }
    //cout << "delete buf: " << buf << "\n";
    string out(buf);
    //cout << "delete out: " << out << "\n";
    return out;
}

string urlth(string url, bool path = false){
  string sep1 = "://";
  string sep2 = "/";
  size_t pos1 = url.find(sep1);
  if (pos1 == -1){
    pos1 = -3;
  }
  size_t pos2 = url.find(sep2, pos1+3);
  string out;
  //cout << "pos1-2 " << pos1 << "-" << pos2 << "\n";
  if (!path){
      out = url.substr(pos1 + 3, pos2 - pos1 - 3);
  }
  else {
      //cout << "in else \n";
      if (pos2 == -1){
          return "";
      }
      out = url.substr(pos2, -1);
      //cout << "out= " << out << "\n";
  }
  //out = regex_replace(out, regex("(\r)"),"");
  out = delete_char(out, '\r');
  return out;
}

int get_http_code(char http_resp[]){
    char numbuf[4] = {0};
    int i = 0;
    while (http_resp[++i] != ' '){}
    numbuf[0] = http_resp[++i];
    numbuf[1] = http_resp[++i];
    numbuf[2] = http_resp[++i];
    //cout << "numbuf = |" << numbuf << "|\n";
    return atoi(numbuf);
}



//void make_GET(char* dest, list<string>::iterator iter)

int isa_main(int argc, char** argv)
{
    cout << "called isa_main with argc=" << argc << ":\n";
    for (int i = 0; i < argc; i++){
      cout << argv[i] << "\n";
    }
    ///////////// param parsing /////////////
    bool par_url = false;
    list <string> url_list;
    bool par_f = false;
    string fpath;
    bool par_c = false;
    string cpath;
    bool par_cc = false;
    string caddr;
    bool par_t = false;
    bool par_a = false;
    bool par_u = false;
    //cout << "argc " << argc << "\n";
    for (int i = 1; i < argc; i++) {
        //cout << "\nargv " << i << " = " << argv[i] << "\n";
        if (strcmp(argv[i], "-f") == 0) {
            //cout << "-f found\n";
            if (par_url || par_f) {
                cerr << "Error: multiple -f or URL specified\n";
                return 1;
            }
            par_f = true;
            i++;
            fpath = argv[i];
            //cout << "fpath set to <" << fpath << ">\n";
        }
        else if (strcmp(argv[i], "-c") == 0) {
            //cout << "-c found\n";
            if (par_c) {
                cerr << "Error: multiple -c specified\n";
                return 1;
            }
            par_c = true;
            i++;
            cpath = argv[i];
            //cout << "cpath set to <" << cpath << ">\n";
        }
        else if (strcmp(argv[i], "-C") == 0) {
            //cout << "-C found\n";
            if (par_cc) {
                cerr << "Error: multiple -C specified\n";
                return 1;
            }
            par_cc = true;
            i++;
            caddr = argv[i];
            //cout << "caddr set to <" << caddr << ">\n";
        }
        else if (strcmp(argv[i], "-T") == 0) {
            //cout << "-T found\n";
            if (par_t) {
                cerr << "Error: multiple -T specified\n";
                return 1;
            }
            par_t = true;
            //cout << "-T set to true\n";
        }
        else if (strcmp(argv[i], "-a") == 0) {
            //cout << "-a found\n";
            if (par_a) {
                cerr << "Error: multiple -a specified\n";
                return 1;
            }
            par_a = true;
            //cout << "-a set to true\n";
        }
        else if (strcmp(argv[i], "-u") == 0) {
            //cout << "-u found\n";
            if (par_u) {
                cerr << "Error: multiple -u specified\n";
                return 1;
            }
            par_u = true;
            //cout << "-u set to true\n";
        }
        else {
            //cout << "no match - assuming url\n";
            if (par_url || par_f) {
                cerr << "Error: multiple -f or URL specified\n";
                return 1;
            }
            par_url = true;
            url_list.push_back(argv[i]);
            //cout << "raw_url set to <" << raw_url[0] << ">\n";
        }

    } // END OF ARGV LOOP
    if (!par_url && !par_f) {
        cerr << "Error: no URL or feedfile specified\n";
        return 1;
    }
    cout << "\nparam parse OK\n\n";

    ///////////// feedfile parsing /////////////
    if (par_f) {
        ifstream fp(fpath);
        if ((!fp.is_open())) {
            cerr << "Error: failed to open feedfile " << fpath << "\n";
            return 2;
        }
        string line;
        cout << "opened feedfile from " << fpath << "\n";
        while (getline(fp, line)) {
            int start = line.find_first_not_of(" \n\r");
            if (start == -1) {
                // empty line
            }
            else {
                if (line[start] == '#') {
                    // comment line
                }
                else {
                    url_list.push_back(line);
                }
            }
        }
        fp.close();
    }
    //cout << "URL list:\n";
    //print_slist(url_list, true, "|");
    //cout << "\n";

    if (url_list.empty()){
      cerr << "Error: feedfile empty\n";
      return 5;
    }


    ///////////// socks /////////////
    list<string>::iterator url_iter;
    string url;
    int sock = 0;
    struct sockaddr_in s_addr;
    char *hostname;
    char hnamebuffer[MAXHOSTNAMELEN];
    char recv_buf[4096] = {0};

    SSL_CTX* ctx;
    SSL*     ssl;
    X509*    server_cert;
    const SSL_METHOD *meth = SSLv23_method();
    if (meth == NULL){
        cout << "Error: ssl method fail\n";
        return 8;
    }

    SSL_library_init();
    //cout << SHA1((const unsigned char*)"innit is british for desu", 42069, NULL);



    for (url_iter = url_list.begin(); url_iter != url_list.end(); ++url_iter) {
        ///////////// get hostname /////////////
        cout << "doing stuff to " << *url_iter << "\n";
        cout << "url host = " << urlth(*url_iter) << "\n";
        cout << "path = " << urlth(*url_iter, true) << "\n";
        s_addr.sin_addr.s_addr = inet_addr((*url_iter).c_str());
        if(s_addr.sin_addr.s_addr != (unsigned)-1) {
        		strcpy(hnamebuffer, urlth(*url_iter).c_str());
        		hostname = hnamebuffer;
            cout << "inet_addr OK, hostname = " << hostname << "\n";
      	}
        else {
            struct hostent *hp = (hostent *)malloc(sizeof(hostent));
            cout << "inet_addr fail, calling gethostbyname()\n";
            hp = gethostbyname(urlth(*url_iter).c_str());
            //cout << hp->h_name << "\n";
            if (hp) {
                cout << "gethostbyname() OK\n";
                bcopy(hp->h_addr, (caddr_t)&s_addr.sin_addr, hp->h_length);
                hostname = hp->h_name;
            }
            else {
                cout << "unknown host: " << *url_iter << "\n";
                continue;
            }
        }
        cout << "got host\n";
        cout << "hostname = " << hostname << "\n";

        ///////////// make socket /////////////
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cerr << "Error: socket creation fail\n";
            return 3;
        }
        cout << "socket creation OK\n\n";
        s_addr.sin_family = AF_INET;

        ///////////// connect /////////////
        // try with port 80 first, if response is 301, switch to 443
        s_addr.sin_port = htons(PORT_HTTP);
        if (connect(sock, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0){
            cerr << "Error: connect fail\n\n";
            //return 6;
            continue;
        }
        cout << "connect OK\n\n";

        ///////////// send GET request /////////////
        string get_string = ("GET " + urlth(*url_iter, true) + " HTTP/1.1\r\nHost: " + hostname + "\r\nConnection: close\r\n\r\n");
        //cout << "request str: \n" << get_string << "\n";
        //get_string = regex_replace(get_string, regex("(\r)"),"\\r");
        char* get_chr = (char *)get_string.c_str();
        //cout << "request chr: \n" << get_chr<< "\n";
        if (send(sock, get_chr, strlen(get_chr), 0) <= 0){
            cout << "send fail\n";
            return 7;
        }
        cout << "send OK\n";
        cout << "sent string: \n|" << get_chr << "|\n\n";

        ///////////// recieve response /////////////
        int recv_cnt = recv(sock, recv_buf, 4096, 0);
        if (recv_cnt < 0){
            cout << "recv fail\n";
            return 8;
        }
        cout << "recv OK";
        if (recv_cnt == 0) {
            cout << " ,closed";
        }
        cout << "\nrecieved string: \n|" << recv_buf << "|\n";
        cout << "response status: " << get_http_code(recv_buf) << "\n\n";
        // 301 -> switch to port 443 and use tls
        // 500 -> unknown domain -> error
        switch(get_http_code(recv_buf)){
            case 200:
                // do nothing here and parse data after the switch
            case 301:
                // switch to port 443
                ///////////// make socket /////////////
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    cerr << "Error: socket creation fail\n";
                    return 3;
                }
                cout << "socket creation OK\n\n";
                s_addr.sin_port = htons(PORT_HTTPS);

                ///////////// connect /////////////
                if (connect(sock, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0){
                    cerr << "Error: connect fail\n\n";
                    //return 6;
                    continue;
                }
                cout << "connect OK\n\n";

                ///////////// tls handshake /////////////
                break;
            case 500:
                // the server doesn't do http i guess - close connection and move on to the next url
                continue;
            default:
                // this shouldn't happen i think
                break;
        } // END OF HTTP RESPONSE SWITCH
    } // END OF URL LOOP

    cout << "\nALL OK\n";




    ///////////// end /////////////
    cout << "=======================DONE=======================\n\n";
    return 0;
}

int main(int argc, char** argv){
    return isa_main(argc, argv);
}
