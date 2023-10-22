import React from 'react'
import './Choice.css';

// this file will hold various types of buttons used throughout the program 
const artist_array = 
[
  {
    "image": "https://i.scdn.co/image/ab6761610000e5ebcdce7620dc940db079bf4952",
    "name": "Ariana Grande",
    "popularity": "87"
  },
  {
    "image": "https://i.scdn.co/image/ab6761610000e5eb5da361915b1fa48895d4f23f",
    "name": "NewJeans",
    "popularity": "83"
  },
];

function Choice({idx}) {
  return (
    <button id="choice" class="button" role="button">
        <img class="img" src={artist_array[idx].image}></img>
    </button>

  )
}
export default Choice