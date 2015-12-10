using UnityEngine;
using System.Collections;
using UnityEngine.Networking;
using UnityStandardAssets.CrossPlatformInput;

public class PlayerController : NetworkBehaviour
{
	private Vector3 input;

	private void Update()
	{
		if(!isLocalPlayer) return;

		input = new Vector3(CrossPlatformInputManager.GetAxis("Horizontal"), 0, CrossPlatformInputManager.GetAxis("Vertical"));
		transform.Translate(input);
	}
}
