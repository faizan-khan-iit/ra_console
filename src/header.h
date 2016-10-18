#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <stack>
#define EXIT_PROGRAM -101010
using namespace std;


struct Error_info{
    int code = 0;
    string info = "invalid query";
};

template<typename T> void printElement(T t, const int& width){
    cout << left << setw(width) << setfill(' ') << t;
}

// Helper functions
string start_up();
int get_input();
void throw_error(int code, string info);
string remove_table_name(string entry);
vector<string> split_string_by_char(string str, string chr);
vector<string> get_cols(string col_line);
vector<string> get_all_cols(string col_line, string table_name);
bool all_same_cols(vector<string> cols1, vector<string> cols2,vector<string> ctypes1, vector<string> ctypes2);
bool any_col_names_same(vector<string>table_cols1, vector<string>table_cols2);
bool repeated_columns(vector<string> col_names);
vector<int> get_order(vector<string> t_cols1, vector<string> t_cols2);
bool valid_par(string query);
bool not_an_alphanumeric(char c);


// Database functions
bool check_if_database_folder(string db_name);
void get_db_info(string db_name);
string get_database();

// Table Functions
vector<string> get_new_table_info(string predicate);
vector<string> get_table_contents(string table_name);
void add_to_all_tables_vector(string table_name);
void show_tables();
int get_table_names(string db_name);
bool valid_table_name(string table_name);
bool valid_column_name(string col_name);
void verify_table_contents(vector<string> table);

// Functions for RA
vector<string> select_ra(string query);
vector<string> project_ra(string query);
vector<string> union_ra(string query);
vector<string> set_diff_ra(string query);
vector<string> cart_prod_ra(string query);
vector<string> rename_ra(string query);

// Query Processing
string get_whole_query();
vector<string> query_parser(string query);
int where_to_split(string query);

// Printing
int newline();
void exit_program();


// SELECT
string convert_to_postfix(string expression);
bool IsOperator(char C);
bool IsOperand(char C);
int get_operator_weight(char C);
int has_higher_precedence(char operator1, char operator2);
vector<int> get_split_points(string predicate, vector<char> valid_splits);
vector<string> get_all_operands(string predicate, vector<int>split_points);
bool is_string(string operand);
bool is_col_name(string operand, vector<string> all_cols);
bool is_int(string operand);
bool is_float(string operand);
bool valid_predicate(string predicate, string table_name, string table_cols, string table_info);
string get_type(string operand, vector<string> vars, vector<string> var_types);
bool valid_operation(string op1_type, string op2_type, string operator_to_use);
string get_result_float(float op_conv1, float op_conv2, string operator1);
string get_result_int(int op_conv1, int op_conv2, string operator1);
string get_result_bool(bool op_conv1, bool op_conv2, string operator1);
string get_result_string(string op_conv1,string op_conv2,string operator1);
string perform_operation(string op1, string op2, string operator1, string data_type, string data_tuple,
    vector<string> all_cols);
bool satisfies_predicate(string predicate, string data_tuple, string table_name, string table_cols, string table_info);

#endif // HEADER_H_INCLUDED
