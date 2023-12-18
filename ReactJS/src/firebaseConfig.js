// Import the functions you need from the SDKs you need
import { initializeApp } from 'firebase/app';
import { getDatabase } from 'firebase/database';

const firebaseConfig = {
    apiKey: "AIzaSyCUDkgzugaNNcNxt77jG6h3e5pO9B8jnDg",
    authDomain: "esp-firebase-3a6d8.firebaseapp.com",
    databaseURL: "https://esp-firebase-3a6d8-default-rtdb.firebaseio.com",
    projectId: "esp-firebase-3a6d8",
    storageBucket: "esp-firebase-3a6d8.appspot.com",
    messagingSenderId: "108071111811",
    appId: "1:108071111811:web:a70fc8f03464b13fb6dfb6"
};

const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

export { database };

