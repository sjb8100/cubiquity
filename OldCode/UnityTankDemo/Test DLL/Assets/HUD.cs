using UnityEngine;
using System.Collections;

public class HUD : MonoBehaviour
{
	private bool voxelsCollideWithEachOther = true;
	private bool voxelsCollideWithWorld = true;
	
	// Use this for initialization
	void Start ()
	{
	
	}
	
	// Update is called once per frame
	void Update ()
	{
		if(Input.GetKeyDown("escape"))
		{
			Application.Quit();
		}
	}
	
	void OnGUI()
	{
		voxelsCollideWithEachOther = GUI.Toggle(new Rect(10, 10, 500, 30), voxelsCollideWithEachOther, "Voxels collide with each other");
		voxelsCollideWithWorld = GUI.Toggle(new Rect(10, 30, 500, 30), voxelsCollideWithWorld, "Voxels collide with the world");
		
		if(GUI.Button(new Rect(10, 60, 200, 30), "Quit"))
		{
			Application.Quit();
		}
		
		Physics.IgnoreLayerCollision(LayerMask.NameToLayer("Cube"), LayerMask.NameToLayer("Cube"), !voxelsCollideWithEachOther);
		Physics.IgnoreLayerCollision(LayerMask.NameToLayer("Cube"), LayerMask.NameToLayer("Volume"), !voxelsCollideWithWorld);
	}
}