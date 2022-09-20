#include "parser.h"

#include <iostream>
#include <vector>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;


typedef struct Bundle
{
	char *name;
	size_t command_count;
	vector<char **> commands;
} Bundle;

vector<Bundle> bundles;

void execute(int bundle_count, bundle_execution *bnd_ex)
{
	//cout << "Execution, bundle_count: " << bundle_count << endl;

	int inp_fd = 0;

	for (int i = 0; i < bundle_count; i++)
	{
		//cout << "Executing " << i << "th bundle." << endl;

		if (i == 0 && bnd_ex[0].input)
			inp_fd = open(bnd_ex[0].input, O_RDONLY);

		//cout << "Input fd: " << inp_fd << endl;
		
		// Find the bundle 
		Bundle bnd;
		for (Bundle bundle : bundles)
		{
			if (!strcmp(bundle.name, bnd_ex[i].name))
			{
				bnd = bundle;
				break;
			}
		}

		//cout << "Fetched bundle: " << bnd.name << endl;
		//cout << "Bundle command_count: " << bnd.command_count << endl;
		
		// Last bundle
		if (i == bundle_count-1)
		{
			//cout << "Last Bundle------" << endl;

			int out_fd = 1;
			if (bnd_ex[i].output)
				out_fd = open(bnd_ex[i].output, O_WRONLY | O_APPEND | O_CREAT, 0666);

			//cout << "Output fd: " << out_fd << endl;

			for (int j = 0; j < bnd.command_count; j++)
			{
				pid_t pid = fork();
				if (pid == 0)
				{
					// Child
					if (inp_fd != 0)
					{
						//cout << "Inp fd updating with: " << inp_fd << endl;
						dup2(inp_fd, 0);
						close(inp_fd);
					}
					if (out_fd != 1)
					{
						//cout << "Out fd updating with: " << out_fd << endl;
						dup2(out_fd, 1);
						close(out_fd);
					}
						
					cout << "Executing " << bnd.commands[j][0] << endl;
					int n = 0;
					char *arg = bnd.commands[j][n];
					
					cout << "Argv: ";
					while (arg)
					{
						cout << arg << " n: " << n << " ";
						arg = bnd.commands[j][++n];
					}
					cout << endl;
					
					if (bnd.commands[j][0][0] == '/')
						execv(bnd.commands[j][0], bnd.commands[j]);
					else
						execvp(bnd.commands[j][0], bnd.commands[j]);
				}
			}
			int st;
			for (int j = 0; j < bnd.command_count; j++)
				wait(&st);

			if (inp_fd != 0) close(inp_fd);
			if (out_fd != 1) close(out_fd);
		}
		else 
		{
			//cout << "Not Last Bundle--------" << endl;

			int pfd[2];
			pipe(pfd);

			//cout << "Pipe created: " << pfd[0] << " " << pfd[1] << endl;

			//cout << "Command count: " << bnd.command_count << endl;

			// Call every command in that bundle
			for (int j = 0; j < bnd.command_count; j++)
			{
				pid_t pid = fork();
				if (pid == 0)
				{
					// Child
					//cout << "Inp_fd value: " << inp_fd << endl;
					if (inp_fd)
					{
						//cout << "Inp fd is updating with: " << inp_fd << endl;
						dup2(inp_fd, 0);
						close(inp_fd);
					}
					dup2(pfd[1], 1);
					close(pfd[0]);
					close(pfd[1]);	

					if (bnd.commands[j][0][0] == '/')
						execv(bnd.commands[j][0], bnd.commands[j]);
					else
						execvp(bnd.commands[j][0], bnd.commands[j]);
				}
			}
			int st;
			for (int j = 0; j < bnd.command_count; j++)
				wait(&st);

			if (inp_fd != 0) close(inp_fd);
			close(pfd[1]);
			inp_fd = pfd[0];
		}
		
		// Erase used bundle
		for (auto it = bundles.begin(); it != bundles.end(); it++)
		{
			if (it->name == bnd.name)
			{
				bundles.erase(it);
				break;
			}
		}
	}
}


int main()
{

	parsed_input *pi = (parsed_input*) malloc(sizeof(parsed_input));
	size_t buffer_size = 255;
	char *buffer = (char*) malloc(buffer_size * sizeof(char));
	int is_bundle_creation = 0;

	Bundle *active_bundle;
	int parse_out;
	size_t input_len;
	while (true)
	{
		input_len = getline(&buffer, &buffer_size, stdin);
		buffer[input_len] = '\0';

		//cout << "Input taken: " << buffer << endl; 

		parse_out = parse(buffer, is_bundle_creation, pi);

		//cout << "Parsed Input: " << endl;
		//cout << "Command type: " << pi->command.type << endl;
		//cout << "Bundle count: " << pi->command.bundle_count << endl;

		if (parse_out)
		{
			// "pbs" fetched, create bundle and close bundle creation
			is_bundle_creation = 0;
			bundles.push_back(*active_bundle);

			//  -----Debug----
			/*
			cout << "Created bundle: " << active_bundle->name << endl;
		       	for (int i = 0; i < active_bundle->commands.size(); i++)
			{
				int n = 0;
				char *arg = active_bundle->commands[i][n];
				while (arg)
				{
					cout << arg << " ";
					n++;
					arg = active_bundle->commands[i][n];
				}
				cout << endl;
			}	

			cout << "Bundles: " << endl;
		       	for (Bundle b : bundles)
			{
				cout << b.name << endl;	
		       		for (int i = 0; i < b.commands.size(); i++)
				{
					int n = 0;
					char *arg = b.commands[i][n];
					while (arg)
					{
						cout << arg << " ";
						n++;
						arg = b.commands[i][n];
					}
					cout << endl;
				}	
			}
			*/
		}

		else if (pi->command.type == PROCESS_BUNDLE_CREATE)
		{
			// pbc command read, creat a Bundle and open bundle creation
			Bundle bundle;
			bundle.name = pi->command.bundle_name;
			bundle.command_count = 0;
			active_bundle = &bundle;
			is_bundle_creation = 1;

			//cout << "Bundle creating name: " << bundle.name << endl;
		}
		else if (pi->command.type == PROCESS_BUNDLE_EXECUTION)
		{
			
			execute(pi->command.bundle_count, pi->command.bundles);

			/*
			for (int i = 0; i < pi->command.bundle_count; i++)
			{
				bundle_execution be = pi->command.bundles[i];
				cout << "{" << be.name << ", ";
				if (be.input) cout << be.input << ", ";
				else cout << "NULL, ";
			        if (be.output) cout << be.output << "}\n";
				else cout << "NULL}\n";	
			}
			*/
		}
		else if (pi->command.type == QUIT)
		{
			exit(0);
		}
		else 
		{
			// This is a command line in a bundle
			active_bundle->command_count++;
			active_bundle->commands.push_back(pi->argv);
		}
	}

	return 0;
}
