import React, { useEffect, useState } from "react";
import { useLocation } from "react-router-dom";
import { useNavigate } from "react-router-dom";
import "./css/Play.css";

function shuffleArray(array) {
  const shuffledArray = [...array];

  for (let i = shuffledArray.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [shuffledArray[i], shuffledArray[j]] = [shuffledArray[j], shuffledArray[i]];
  }

  return shuffledArray;
}

const Play = () => {
  const location = useLocation();
  const difficulty = location.state?.difficulty;
  const [jsonData, setJsonData] = useState([]);
  const [loading, setLoading] = useState(true);

  const [error, setError] = useState(null);
  const [showPopularity, setShowPopularity] = useState(false);

  const navigate = useNavigate();

  useEffect(() => {
    const fetchData = async () => {
      try {
        const s = `http://localhost:5555/top_artists?difficulty=${difficulty}`;
        const response = await fetch(s);
        console.log(s);

        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }

        const data = await response.json();
        console.log(data);

        setJsonData(shuffleArray(data));
      } catch (error) {
        setError(error.message);
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, [difficulty]);

  const handleClick = async (res) => {
    if (jsonData.length < 2) return;

    if (res === false || jsonData.length === 2) {
      setShowPopularity(true);
      await new Promise((resolve) => setTimeout(resolve, 2500));
      navigate("/results", { state: { res } });
    } else {
      const lessPopularIndex = pops[0] > pops[1] ? 1 : 0;

      const updatedList = jsonData.filter(
        (_, index) => index !== lessPopularIndex,
      );
      setJsonData(updatedList);
    }
  };

  if (loading) return <div>Loading...</div>;
  if (error) return <div>Error: {error}</div>;

  const artists =
    jsonData.length > 0
      ? [jsonData[0].name, jsonData[1] ? jsonData[1].name : ""]
      : ["", ""];
  const images =
    jsonData.length > 0
      ? [jsonData[0].image, jsonData[1] ? jsonData[1].image : ""]
      : ["", ""];
  const pops =
    jsonData.length > 0
      ? [jsonData[0].popularity, jsonData[1] ? jsonData[1].popularity : 0]
      : [0, 0];

  return (
    <div>
      <h1 className="question-text"> Who do you think is more popular? </h1>
      <div className="container">
        <div className="button-container">
          <button
            className="button"
            onClick={() => handleClick(pops[0] >= pops[1])}
          >
            <img className="img" src={images[0]} alt={artists[0]} />
            <p>{artists[0]}</p>
          </button>
          <div className="green-square">{showPopularity ? pops[0] : null}</div>
        </div>
        {jsonData[1] && (
          <div className="button-container">
            <button
              className="button"
              onClick={() => handleClick(pops[1] >= pops[0])}
            >
              <img className="img" src={images[1]} alt={artists[1]} />
              <p>{artists[1]}</p>
            </button>
            <div className="green-square">
              {showPopularity ? pops[1] : null}
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default Play;
