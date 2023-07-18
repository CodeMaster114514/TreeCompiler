#include "compiler.h"

expressionable_operator_precedence_group
	operator_precendence[TOTAL_OPERATOR_GROUPS] =
		{
			{// lever 1
			 .operators =
				 {
					 "++",
					 "--",
					 "()",
					 "[]",
					 "{}",
					 ".",
					 "->",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 2
			 .operators =
				 {
					 "*",
					 "/",
					 "%",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 3
			 .operators =
				 {
					 "+",
					 "-",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 4
			 .operators =
				 {
					 "<<",
					 ">>",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 5
			 .operators =
				 {
					 "<",
					 "<=",
					 ">",
					 ">=",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 6
			 .operators =
				 {
					 "==",
					 "!=",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 7
			 .operators =
				 {
					 "&",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 8
			 .operators =
				 {
					 "^",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 9
			 .operators =
				 {
					 "|",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 10
			 .operators =
				 {
					 "&&",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 11
			 .operators =
				 {
					 "||",
					 NULL},
			 .associativity = LEFT_TO_RIGHT},
			{// lever 12
			 .operators =
				 {
					 "?",
					 ":",
					 NULL},
			 .associativity = RIGHT_TO_LEFT},
			{// lever 13
			 .operators =
				 {
					 "=",
					 "*=",
					 "/=",
					 "%=",
					 "+=",
					 "-=",
					 "<<=",
					 ">>=",
					 "&=",
					 "^=",
					 "|=",
					 NULL},
			 .associativity = RIGHT_TO_LEFT},
			{// lever 14
			 .operators =
				 {
					 ",",
					 NULL},
			 .associativity = LEFT_TO_RIGHT}};
