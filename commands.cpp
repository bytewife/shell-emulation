// $Id: commands.cpp,v 1.19 2020-10-20 18:23:13-07 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   {"#"     , fn_ignore},
   {"^D"    , fn_exit},
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
cout << "hihihi";
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   // DONE
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string filename = "";
   string err = "No such file or directory";
   if (words.size() < 2) { cout << err << endl; return; }
   err = "cat: " + words.at(1) + ": " + err;
   try { 
      auto dir = state.get_inode_ptr_from_path(words.at(1), filename);
      // find first
      auto dirents = dir->get_contents()->get_dirents();
      if (dirents.find(filename) == dirents.end()) {
         throw file_error("Going to catch"); };
      auto toCat = dir->get_contents()->get_dirents()[filename];
      for ( auto i : toCat->get_contents()->readfile()) {
         cout << i << " "; }
      cout << endl;
   }
   catch(std::exception const& e) {
      cout << err << endl; return; }
}

void fn_cd (inode_state& state, const wordvec& words){
   // DONEa
   auto err = "Please specify directory name. No plain files.";
   string dirname = "";
   if (words.size() < 2) { state.set_cwd(state.get_root()); return; }
   try {
      auto dir = state.get_inode_ptr_from_path(words.at(1), dirname);
      auto dirents = dir->get_contents()->get_dirents();
      if (dirname == "/") { state.set_cwd(state.get_root()); return; }
      if (dirents.find(dirname) == dirents.end()) {
         throw file_error("Going to catch"); };
      auto toCd = dirents[dirname];
      state.set_cwd(toCd);
      // state.set_cwd(dir);
   }
   catch(std::exception const& e) {
      cout << err << endl; return; }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   // call dtor here
   // state.~inode_state();
   state.get_cwd() = state.get_root();
   state.get_root()->get_contents()->recur_rmr();
   state.get_root()->get_contents()->get_dirents().erase(".");
   state.get_root()->get_contents()->get_dirents().erase("..");
   state.get_root()->get_contents() = nullptr;
   state.get_root() = nullptr;

   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   // DONE
   auto err = "Please specify directory name. No plain files.";
   string dirname = "";
   try {
      if (words.size() < 2) { 
         state.get_cwd()->get_contents()->print_dirents(); return; }
      auto dir = state.get_inode_ptr_from_path(words.at(1), dirname);
      auto dirents = dir->get_contents()->get_dirents();
      if (dirname == "/") { state.get_root()->get_contents()->print_dirents(); 
         return; }
      if (dirents.find(dirname) == dirents.end()) {
         throw file_error("Going to catch"); };
      auto toLs = dirents[dirname];
      // Error check that it's a directory. Add this to all of the above
      toLs = toLs->get_contents()->get_dirents()["."];

      toLs->get_contents()->print_dirents();
   }
   catch(std::exception const& e) {
      cout << err << endl; return; }
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_lsr (inode_state& state, const wordvec& words){
   auto err = "Please specify directory name. No plain files.";
   auto dirToLsr = state.get_root();
   if (words.size() > 1) {
      string dirname = "";
      try {
         auto dir = state.get_inode_ptr_from_path(words.at(1), dirname);
         auto dirents = dir->get_contents()->get_dirents();
         if (dirname == "/") { dirname = "."; }
         if (dirents.find(dirname) == dirents.end()) {
            throw file_error("Going to catch"); };
         dirToLsr = dirents[dirname];
         // Error check that it's a directory. commenting it might have broken it?
         // dirToLsr = dirents["."];
      }
      catch(std::exception const& e) {
         cout << err << endl; return; }
   }
   dirToLsr->get_contents()->recur_lsr();
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   // DONE
   auto err = "Please specify file name. No directories.";
   if (words.size() < 2) { cout << err << endl; return; }
   try
   {
      string back_name = "";
      auto toMake = state.get_inode_ptr_from_path(words.at(1), back_name);
      auto existing_file_dirents = toMake->get_contents()
         ->get_dirents();
      // auto back_name = split(words.at(1), "/").back();
      if(existing_file_dirents.find(back_name) == 
         existing_file_dirents.end()) 
      { 
         toMake->get_contents()->mkfile(back_name)
            ->get_contents()->writefile(words); }
      else { existing_file_dirents[back_name]->get_contents()
            ->writefile(words); }
   }
   catch(std::exception const& e) {
      cout << err << endl;
   }

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   // TODO check duplicates and then err
   string back_name = "";
   try {
      auto toMakeIn = state.get_inode_ptr_from_path(words.at(1), back_name);
      auto dirents = toMakeIn->get_contents()->get_dirents();
      if (dirents.find(back_name) == dirents.end()) { // If doesn't exist
         toMakeIn->get_contents()->mkdir(back_name);
      }
      else { cout << "Directory already exists." << endl; };
   }
   catch(std::exception const& e) {
      cout << "Directory path does not exist." << endl; 
   }
   // TODO parse path
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_prompt (inode_state& state, const wordvec& words){
   string concat = "";
   for (size_t i = 1; i < words.size(); i++) { 
      concat += words.at(i); 
      concat += " ";
   }
   if (concat ==  "") concat = " ";
   state.prompt(concat);
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   auto toPrint = state.get_cwd()->get_contents()->get_path();
   if (state.get_cwd() != state.get_root()) { 
      toPrint = toPrint.substr(0, toPrint.size()-1); }
   cout << toPrint << endl;
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   auto err = "File does not exist.";
   string toDelete = "";
   try {
      auto toDeleteFrom = state.get_inode_ptr_from_path(words.at(1), toDelete);
      if (toDelete == ".." || toDelete == "/") 
         { throw file_error("Going to catch"); }
      toDeleteFrom->get_contents()->remove(toDelete); // handles file confirming
   }
   catch(std::exception const& e) {
      cout << err << endl; return; }
   // 
}

void fn_rmr (inode_state& state, const wordvec& words){
   // im assuming its like rmr
   auto err = "Directory path does not exist.";
   string toDelete = "";
   try {
      auto toDeleteFrom = state.get_inode_ptr_from_path(words.at(1), toDelete);
      if (toDelete == ".." || toDelete == "/") 
         { throw file_error("Going to catch"); }
      toDeleteFrom->get_contents()->rmr(toDelete); // handles file confirming
      }
   catch(std::exception const& e) {
      cout << err << endl; return; }

   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_ignore (inode_state& state, const wordvec& words){
   return;
}
   

