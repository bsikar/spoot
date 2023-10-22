import React from "react";

const Home = () => {
  const handleButtonClick = (difficulty) => {
    window.location.href = `http://localhost:5555/login?difficulty=${difficulty}`;
  };

  return (
    <div className="home-container">
      <h1>Welcome</h1>
      <p>
        This game will customize a series of questions depending on your music
        taste.{" "}
      </p>
      <p>You will select between two choices and gradually work your way up.</p>
      <p>Click below to login and begin the game!</p>
      <div className="buttons-row">
        <button
          className="welcomeButton"
          onClick={() => handleButtonClick("easy")}
        >
          EASY
        </button>
        <button
          className="welcomeButton"
          onClick={() => handleButtonClick("medium")}
        >
          MEDIUM
        </button>
        <button
          className="welcomeButton"
          onClick={() => handleButtonClick("hard")}
        >
          HARD
        </button>
      </div>
    </div>
  );
};

export default Home;
