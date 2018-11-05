#define GAMELIFT_USE_STD
#include <aws\gamelift\server\GameLiftServerAPI.h> 
#include <iostream>

using namespace std;

int main()
{
	Aws::GameLift::Server::InitSDKOutcome initOutcome = Aws::GameLift::Server::InitSDK();
	if (initOutcome.IsSuccess())
	{
		cout << "NetLog: AWS GameLift API: Initialization success!" << endl;
	}
	else
	{
		cout << "NetLog: AWS GameLift API:" << initOutcome.GetError().GetErrorMessage() << endl;
	}

	Aws::GameLift::GenericOutcome outcome = Aws::GameLift::Server::ProcessEnding();
	if (outcome.IsSuccess())
	{
		cout << "NetLog: AWS GameLift API: Shutting down..." << endl;
	}
	else
	{
		cout << "NetLog: AWS GameLift API:" << outcome.GetError().GetErrorMessage() << endl;
	}

	system("pause");
	return 0;
}