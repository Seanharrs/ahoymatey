using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;

public class GameNetworkingManager : NetworkManager
{
	public void StartGameHost()
	{
		Debug.Log("Host starting at: " + Time.timeSinceLevelLoad);
		StartHost();
		FindObjectOfType<Button>().gameObject.SetActive(false);
	}

	public override void OnStartHost()
	{
		base.OnStartHost();
		Debug.Log("Host started at: " + Time.timeSinceLevelLoad);
	}

	public override void OnStartClient(NetworkClient networkClient)
	{
		base.OnStartClient(networkClient);
		Debug.Log("Client started at: " + Time.timeSinceLevelLoad);
	}

	public override void OnClientConnect(NetworkConnection connection)
	{
		base.OnClientConnect(connection);
		Debug.Log("Client connected at: " + Time.timeSinceLevelLoad + " to address " + connection.address);
	}
}
