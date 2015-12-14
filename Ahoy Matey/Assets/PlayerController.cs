using UnityEngine;
using UnityEngine.Networking;
using UnityStandardAssets.CrossPlatformInput;

public class PlayerController : NetworkBehaviour
{
	private Vector3 input;

	private void Update()
	{
		if(!isLocalPlayer) return;

		input = new Vector3
		{
			x = CrossPlatformInputManager.GetAxis("Horizontal"),
			y = 0,
			z = CrossPlatformInputManager.GetAxis("Vertical")
		};
		transform.Translate(input);
	}

	public override void OnStartLocalPlayer() { GetComponentInChildren<Camera>().enabled = true; }
}
