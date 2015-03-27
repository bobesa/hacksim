#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>

#define DEFAULT_ACCELERATION 3
#define RETURN_KEY_CHAR 13
#define QUIT_KEY_CHAR 'q'
#define MAX_CLASS_MEMBERS 15
#define MAX_CLASS_FUNCTIONS 8
#define MAX_ARGUMENTS 8
#define MAX_CONTENT_LINES 50
#define NewTab "\t"
#define NewLine makeNL(offset)

namespace hacksim {
    
    enum {
        SIM_STYLE_CLASS,
        SIM_STYLE_FUNCTION,
        SIM_STYLE_CONTENT,
        
        SIM_STYLE_COUNT
    };
    
    enum {
        SIM_TYPE_VOID,
        SIM_TYPE_NUMBER,
        SIM_TYPE_BOOL,
        SIM_TYPE_STRING,
        
        SIM_TYPE_COUNT
    };
    
    enum {
        PROCESSING_TYPE_EMPTY,
        PROCESSING_TYPE_LOADING,
        PROCESSING_TYPE_BINARY,
        
        PROCESSING_TYPE_COUNT
    };
    
#pragma mark -
#pragma mark Random operations
    
    /**
     Returns new line with offset
     @return {std::string} new line string with offset
     */
    static std::string makeNL(int offset) {
        std::ostringstream out;
        out << std::endl;
        for(int i = 0; i < offset; i++) out << NewTab;
        return out.str();
    }
    
    //Math operations
    static bool getRandomBool() { return arc4random() % 2 == 0; }
    static int getRandomInt(int max) {
        return arc4random() % (max);
    }
    static int getRandomInt() { return getRandomInt(arc4random()); }
    
#pragma mark -
#pragma mark Processing BaseClass
    
    class HackSimProcessing {
    public:
        HackSimProcessing(){};
        virtual ~HackSimProcessing(){};
        virtual void run() = 0;
    };
    
#pragma mark -
#pragma mark Processings
    
    class HackSimProcessingBinary : public HackSimProcessing {
    public:
        HackSimProcessingBinary() : HackSimProcessing() {};
        ~HackSimProcessingBinary(){};
        
        void run() {
            int cols = 8 + getRandomInt(22), rows = 10 + getRandomInt(100), speed = getRandomInt(10);
            
            std::cout
            << std::showbase // show the 0x prefix
            << std::internal // fill between the prefix and the number
            << std::setfill('0');
            
            for(int r = 0; r < rows; r++) {
                for(int c = 0; c < cols; c++) {
                    std::cout << std::setw(4) << std::hex << getRandomInt(128) << " ";
                }
                std::cout << std::endl;
                usleep(2500 + (speed * 500));
            }
            
        }
    };
    
    class HackSimProcessingLoading : public HackSimProcessing {
    public:
        HackSimProcessingLoading() : HackSimProcessing() {};
        ~HackSimProcessingLoading(){};
        
        void run() {
            int max = 10 + getRandomInt(100000), i = 0, speed = getRandomInt(5);
            
            std::string wording = "Loading";
            std::string empty = getRandomBool() ? "_" : " ";
            std::string filled = ".";
            
            bool showPercents = getRandomBool();
            
            switch (getRandomInt(3)) {
                case 0: wording = "Loading"; break;
                case 1: wording = "Download"; break;
                case 2: wording = "Processing"; break;
            }
            
            switch (getRandomInt(5)) {
                case 0: filled = "."; break;
                case 1: filled = "x"; break;
                case 2: filled = "X"; break;
                case 3: filled = "~"; break;
                case 4: filled = "0"; break;
            }
            
            for(int i = 0; i <= 100; i++) {
                float progress = ((float)max / (float)100) * i;
                
                std::cout << std::setw(0) << std::setprecision(2);
                std::cout << wording << " [";
                for(int c = 0; c < 100; c++) std::cout << (i > c ? filled : empty);
                std::cout << "] ";
                if(showPercents) {
                    std::cout << std::setprecision(2) << std::fixed << i << "%";
                } else {
                    std::cout << std::setprecision(0) << std::fixed << progress << " / " << max;
                }
                std::cout << '\r';
                std::cout.flush();
                usleep(2500 + (speed * 500));
            }
        }
        
    };
    
#pragma mark -
#pragma mark BaseClass
    
    class HackSim {
        friend class HackSimLangC;
    private:
        int acceleration;
        std::vector<std::string> names;
        std::vector<std::string> values;
        std::vector<std::string> comments;
        
    public:
        HackSim(int acceleration) : acceleration(acceleration) {}
        ~HackSim(){}
        
        /**
         Will start typing session
         */
        void start() {
            // Prepare simulation
            std::string out = generate(getRandomInt(SIM_TYPE_COUNT),0);
            std::string::iterator i = out.begin();
            while(true) {
                system("stty raw"); // Reset terminal to normal "cooked" mode
                char input = getchar(); // Wait for single character
                if(input == QUIT_KEY_CHAR) break; // Quit if q was pressed
                system("stty cooked -echo"); // Reset terminal to normal "cooked" mode
                
                //Check for RETURN key
                if(input == RETURN_KEY_CHAR) {
                    //Print rest of code that should be wrote
                    while(i != out.end()) {
                        std::cout << *i;
                        std::cout.flush();
                        usleep(100);
                        i++;
                    }
                    
                    //Run processing object
                    HackSimProcessing* p = NULL;
                    switch(getRandomInt(PROCESSING_TYPE_COUNT)) {
                        case PROCESSING_TYPE_LOADING: p = new HackSimProcessingLoading(); break;
                        case PROCESSING_TYPE_BINARY: p = new HackSimProcessingBinary(); break;
                    }
                    if(p != NULL) {
                        printf("\n\n");
                        p->run();
                        delete p;
                    }
                }
                
                //Print next char
                for(int c = 0; c < acceleration && i != out.end(); c++) {
                    std::cout << *i;
                    std::cout.flush();
                    usleep(100);
                    i++;
                }
                
                //Generate new code
                if(i == out.end()) {
                    out = generate(getRandomInt(SIM_TYPE_COUNT),0);
                    i = out.begin();
                    printf("\n\n");
                }
            }
            this->end();
        }
        
        /**
         Will end typing session
         */
        void end() {
            system("stty cooked"); // Reset terminal to normal "cooked" mode
            printf("\n");
        }
        
        //Abstract methods
        virtual std::string getType(bool canBePointer, bool canBeVoid) = 0;
        virtual std::string getArguments() = 0;
        virtual std::string getComment() = 0;
        virtual std::string generate(int type, int offset) = 0;
        
        //Collections
        virtual std::string getRandomValue() = 0;
        std::string getRandomName() { return names[getRandomInt((int)names.size())]; }
    };
    
#pragma mark -
#pragma mark Language: C
    
    class HackSimLangC : public HackSim {
    public:
        HackSimLangC(int acceleration) : HackSim(acceleration) {
            //Prepare names
            names.push_back("doSomething");
            names.push_back("computate");
            names.push_back("consult");
            names.push_back("getSomething");
            names.push_back("prepare_it");
            names.push_back("communicate_with_server");
            names.push_back("request");
            names.push_back("propagate");
            names.push_back("increase");
            names.push_back("decrease");
            names.push_back("camelize");
            names.push_back("galvanize");
            names.push_back("init");
            names.push_back("main");
            names.push_back("prep");
            
            //Prepare values
            values.push_back("\"something\"");
            values.push_back("\"value\"");
            values.push_back("\"success\"");
            values.push_back("\"fail\"");
            
            //Prepare comments
            comments.push_back("Prepare this magnificent stuff!");
            comments.push_back("Increse delay of something something!");
            comments.push_back("It's a me! Mario!");
            comments.push_back("TODO: Actually write real meaningful code");
            comments.push_back("FIXME: Or not?");
            comments.push_back("Amazing story of no tests.");
            comments.push_back("Gingerbread is soooo sweet...");
            
        }
        
        /**
         Returns value
         @return {std::string} value string
         */
        std::string getRandomValue() {
            std::ostringstream out;
            switch(getRandomInt(SIM_TYPE_COUNT)) {
                case SIM_TYPE_BOOL: out << (getRandomBool() ? "true" : "false"); break;
                case SIM_TYPE_NUMBER: out << getRandomInt(); break;
                case SIM_TYPE_STRING: out << values[getRandomInt((int)values.size())]; break;
                default: out << "NULL";
            }
            return out.str();
        }
        
        /**
         Returns type
         @return {std::string} string with type
         */
        std::string getType(bool canBePointer, bool canBeVoid) {
            int t = getRandomInt(SIM_TYPE_COUNT);
            while(!canBeVoid && t == SIM_TYPE_VOID) t = getRandomInt(SIM_TYPE_COUNT);
            
            std::ostringstream out;
            switch(t) {
                case SIM_TYPE_VOID: out << "void"; break;
                case SIM_TYPE_BOOL: out << "bool"; break;
                case SIM_TYPE_STRING: out << "string"; break;
                    
                default:
                case SIM_TYPE_NUMBER: out << "int"; break;
            }
            if(canBePointer && t != SIM_TYPE_VOID && getRandomBool()) out << "*";
            return out.str();
        }
        
        /**
         Returns comment (or not)
         @return {std::string} string with comments
         */
        std::string getComment() {
            if(getRandomBool()) return ""; //Skip comment
            std::ostringstream out;
            out << " // " << comments[getRandomInt((int)comments.size())];
            return out.str();
        }
        
        /**
         Returns arguments
         @return {std::string} string with arguments
         */
        std::string getArguments() {
            std::ostringstream out;
            int argc = getRandomInt(MAX_ARGUMENTS);
            for(int a = 0; a < argc; a++) {
                out << getType(true,false) << " " << getRandomName();
                if(a != argc - 1) out << ", ";
            }
            return out.str();
        }
        
        /**
         Returns randomized code
         @param {int} type content type
         @param {int} offset content offset
         @return {std::string} string with code
         */
        std::string generate(int type, int offset) {
            std::ostringstream out;
            switch(type) {
                case SIM_STYLE_CLASS: {
                    int argumentCount = getRandomInt(MAX_CLASS_MEMBERS), functionCount = getRandomInt(MAX_CLASS_FUNCTIONS);
                    std::string className = getRandomName();
                    out << "class " << className << " {" << NewLine;
                    out << NewTab << className << "();" << NewLine;
                    out << NewTab << "~" << className << "();" << NewLine;
                    if(functionCount > 0) {
                        out << NewLine;
                        for(int i = 0; i < functionCount; i++) out << NewTab << generate(SIM_STYLE_FUNCTION,offset+1) << NewLine;
                    }
                    if(argumentCount > 0) {
                        out << NewLine;
                        for(int i = 0; i < argumentCount; i++) out << NewTab << generate(SIM_STYLE_CONTENT,offset+1) << NewLine;
                    }
                    out << "}";
                } break;
                    
                case SIM_STYLE_FUNCTION:
                    out << getType(false,true) << " " << getRandomName() << "(" << getArguments() << ") {" << NewLine;
                    for(int i = 0; i < getRandomInt(MAX_CONTENT_LINES); i++) {
                        out << NewTab << generate(SIM_STYLE_CONTENT,offset+1) << NewLine;
                    }
                    out << "}";
                    break;
                    
                case SIM_STYLE_CONTENT:
                    out << getType(true,false) << " " << getRandomName() << " = " << getRandomValue() << ";" << getComment();
                    break;
            }
            return out.str();
        }
        
    };
    
}

#pragma mark -
#pragma mark Main

int main(int argc, char** argv) {
    int acceleration = argc >= 2 ? atoi(argv[1]) : DEFAULT_ACCELERATION;
    
    hacksim::HackSim* sim = new hacksim::HackSimLangC(acceleration);
    sim->start();
}
