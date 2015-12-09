using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;

public class GameNetworkingManager : NetworkManager
{
	public void StartGameHost()
	{
		StartHost();
		Debug.Log("Starting: " + Time.timeSinceLevelLoad);
		FindObjectOfType<Button>().gameObject.SetActive(false);
	}
	
	public override void OnStartHost()
	{
		base.OnStartHost();
		Debug.Log("Started: " + Time.timeSinceLevelLoad);
	}
}
