import React from "react";

const Home = () => {
  const handleButtonClick = (difficulty) => {
    window.location.href = `http://localhost:5555/login?difficulty=${difficulty}`;
  };

  return (
    <div className="home-container">
      <h1>Welcome</h1>
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
