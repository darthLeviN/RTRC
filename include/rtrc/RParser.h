/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

 /*
  * File:   rtrc_Parser.h
  * Author: roohi
  *
  * Created on June 24, 2019, 7:22 PM
  */

#ifndef rtrc_PARSER_H
#define rtrc_PARSER_H

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>



namespace rtrc
{
	class RParser
	{
	public:
		// Parses a new instance.
		RParser(int argv, char* argc[]);
		// Parses a new instance.
		RParser(char* lcmd);


		// parses more props. calls set prop event of Instance 
		void Parse_set_props(int argv, char* argc[]);
		void Parse_set_props_TS(int argv, char* argc[]); // used for thread safety.
		// parses more props. calls set prop event of Instance.
		void call_cmd(char* lcmd);
		void call_cmd_TS(char* lcmd); // used for thread safety.

		struct ParsingState
		{
			enum onGoingJob
			{
				encapsulation, clearingWhitespace
			};
		};

	private:
		// parses a launch command line arguments directly.
		void Parse_launch(int argv, char* argc[]);
		// parses a launch command string
		void Parse_launch(char* lcmd);

		std::mutex um; // universal mutex to lock the whole object.
		//std::unique_ptr<Instance> instance;

	};
}

#endif /* rtrc_PARSER_H */

