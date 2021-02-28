using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Lean.Transition;

/// <summary>
/// Class Chip represents one chip of the board
/// </summary>
public class Chip : MonoBehaviour
{

    // Indicator if the chip is white or black
    public bool isWhite;

    // Default position of the chip used for reset after animation
    Vector3 defaultPosition;


    // Start is called before the first frame update
    void Start()
    {
       if (!isWhite) transform.Rotate(Vector3.right * 180);   
    }

    // Update is called once per frame
    void Update()
    {
       
    }

    public void updateDefPosition()
    {
        defaultPosition = transform.localPosition;
    }

    /// <summary>
    /// Animates the flip of the chip and switches the value of the isWhite varibale accordingly
    /// </summary>
    public void flip()
    {
        isWhite = !isWhite; 
        transform.localPositionTransition(new Vector3(0, 0, 0.02f) + defaultPosition, 0.3f).
        JoinTransition().
        transform.RotateTransition(new Vector3(180.0f, 0.0f, 0.0f), 0.3f).
        JoinTransition().
        transform.localPositionTransition(defaultPosition, 0.3f);
    }
}
