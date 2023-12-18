import { useEffect, useState } from 'react';
import { ref, get } from 'firebase/database';
import { database } from './firebaseConfig';
import "./App.css";

const App = () => {
  const [distance1, setDistance1] = useState(null);
  const [distance2, setDistance2] = useState(null);
  const [distance3, setDistance3] = useState(null);

  useEffect(() => {
    fetchData();

    const intervalId = setInterval(() => {
      fetchData();
    }, 100);

    return () => {
      clearInterval(intervalId);
    };
  }, []);

  const fetchData = async () => {
    try {
      const distances = ['Distance_1', 'Distance_2', 'Distance_3'];

      distances.forEach(async (distance, index) => {
        const distanceRef = ref(database, `${distance}/Value`);
        const snapshot = await get(distanceRef);

        if (snapshot.exists()) {
          switch (index) {
            case 0:
              setDistance1(snapshot.val());
              break;
            case 1:
              setDistance2(snapshot.val());
              break;
            case 2:
              setDistance3(snapshot.val());
              break;
            default:
              break;
          }
        } else {
          console.error(`${distance} document does not exist!`);
        }
      });
    } catch (error) {
      console.error("Error fetching data:", error.message);
    }
  };

  const renderImageOrText = (distance) => {
    if (distance && distance < 20) {
      return (
        <div className="parking_slot show">
          <div className="img show"></div>
        </div>
      );
    } else {
      return (
        <div className="parking_slot">
          <div className="text">Available</div>
        </div>
      );
    }
  };

  return (
    <div className="parking_container">
      <h1>Parking Slot Status</h1>
      <div className="container">
        {renderImageOrText(distance1)}
        {renderImageOrText(distance2)}
        {renderImageOrText(distance3)}
      </div>
    </div>
  );
};

export default App;
