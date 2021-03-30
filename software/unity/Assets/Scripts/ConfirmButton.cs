﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
using TMPro;

public class ConfirmButton : MonoBehaviour
{
    public GameManager mGameManager;
    public Board mBoard;
    // public PieceManager mPieceManager;
    
    // Start is called before the first frame update
    void Start()
    {
        GetComponent<Button>().interactable = false;
    }

    // Logic to decide when to enable this confirm button
    void FixedUpdate()
    {
        if (mGameManager.mWaiting)
        {
            GetComponent<Button>().interactable = false;
        }
        else
        {
            switch (GlobalState.GameState)
            {
                case GameState.Placement:
                    // Make confirm button visible
                    GetComponent<Image>().enabled = true;
                    GetComponentInChildren<TextMeshProUGUI>().enabled = true;
                    // GetComponent<Button>().interactable = (mPieceManager.mNoActiveInteraction && mBoard.ValidateShips());
                    break;
                case GameState.Attacking:
                    // Make confirm button visible
                    GetComponent<Image>().enabled = true;
                    GetComponentInChildren<TextMeshProUGUI>().enabled = true;
                    GetComponent<Button>().interactable = (mBoard.mTargetedCell != null);
                    break;
                case GameState.Defending:
                    // Hide confirm button
                    GetComponent<Image>().enabled = false;
                    GetComponentInChildren<TextMeshProUGUI>().enabled = false;
                    break;
                default:
                    Debug.Log("ERROR: This should not be possible!");
                    break;
            }
        }
    }
}
