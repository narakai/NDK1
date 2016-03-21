package com.example.leon.ndk1;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.google.common.base.Function;
import com.google.common.base.Joiner;
import com.google.common.collect.Lists;
import com.google.common.primitives.Ints;

import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

/**
 * A placeholder fragment containing a simple view.
 */
public class StoreActivityFragment extends Fragment implements StoreListener{

    private EditText mUIKeyEdit, mUIValueEdit;
    private Spinner mUITypeSpinner;
    private Button mUIGetButton, mUISetButton;
    private Pattern mKeyPattern;

    private StoreThreadSafe mStore = new StoreThreadSafe(this);
    private long mWatcher;

    public StoreActivityFragment() {
    }

    @Override
    public void onResume() {
        super.onResume();
        mWatcher = mStore.startWatcher();
    }

    @Override
    public void onPause() {
        super.onPause();
        mStore.stopWatcher(mWatcher);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {


        View rootView = inflater.inflate(R.layout.fragment_store, container, false);

        mKeyPattern = Pattern.compile("\\p{Alnum}+");
        mUIKeyEdit = (EditText) rootView.findViewById(R.id.uiKeyEdit);
        mUIValueEdit = (EditText) rootView.findViewById(R.id.uiValueEdit);

        ArrayAdapter<StoreType> adapter = new ArrayAdapter<StoreType>(getActivity(), android.R.layout.simple_spinner_dropdown_item, StoreType.values());
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mUITypeSpinner = (Spinner) rootView.findViewById(R.id.uiTypeSpinner);
        mUITypeSpinner.setAdapter(adapter);

        mUIGetButton = (Button) rootView.findViewById(R.id.uiGetValueButton);
        mUIGetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onGetValue();
            }
        });

        mUISetButton = (Button) rootView.findViewById(R.id.uiSetValueButton);
        mUISetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onSetValue();
            }
        });

        return rootView;
    }

    private void onSetValue() {
        String key = mUIKeyEdit.getText().toString();
        String value = mUIValueEdit.getText().toString();
        StoreType type = (StoreType) mUITypeSpinner.getSelectedItem();
        if (!mKeyPattern.matcher(key).matches()) {
            displayMessage("Incorrect Key");
            return;
        }
        try {
            switch (type) {
                case IntegerArray:
                    //得到list
                    List<Integer> integerList = stringToList(new Function<String, Integer>() {
                        @Override
                        public Integer apply(String input) {
                            return Integer.parseInt(input);
                        }
                    }, value);
                    //Guava offers many useful methods to deal with primitives and arrays
                    mStore.setIntegerArray(key, Ints.toArray(integerList));
                    break;
                case StringArray:
                    String[] stringArray = value.split(";");
                    mStore.setStringArray(key, stringArray);
                    break;
                case ColorArray:
                    List<Color> colorList = stringToList(new Function<String, Color>() {
                        @Override
                        public Color apply(String input) {
                            return new Color(input);
                        }
                    }, value);
                    mStore.setColorArray(key, colorList.toArray(new Color[colorList.size()]));
                    break;
                case Color:
                    mStore.setColor(key, new Color(value));
                    break;
                case Boolean:
                    if (value.equals("true") || value.equals("1")) {
                        mStore.setBoolean(key, true);
                    } else if (value.equals("false")
                            || value.equals("0")) {
                        mStore.setBoolean(key, false);
                    } else {
                        throw new IllegalArgumentException();
                    }
                    break;
                case Integer:
                    mStore.setInteger(key, Integer.parseInt(value));
                    break;
                case String:
                    //Insert or update string entries in the store
                    mStore.setString(key, value);
                    break;
            }
        } catch (NumberFormatException eNumberFormatException) {
            displayMessage("Incorrect value.");
        } catch (StoreFullException eStoreFullException) {
            displayMessage(eStoreFullException.getMessage());
        } catch (Exception e) {
            displayMessage("Incorrect value");
        }

    }

    //convert a string into a list of the target type
    private <T> List<T> stringToList(Function<String, T> pConversion, String pValue) {
        String[] splitArray = pValue.split(";");
        List<String> splitList = Arrays.asList(splitArray);
        return Lists.transform(splitList, pConversion);
    }

    private void onGetValue() {
        String key = mUIKeyEdit.getText().toString();
        StoreType type = (StoreType) mUITypeSpinner.getSelectedItem();

        if (!mKeyPattern.matcher(key).matches()) {
            displayMessage("Incorrect key");
            return;
        }
        try {
            switch (type) {
                case IntegerArray:
                    mUIValueEdit.setText(Ints.join(";", mStore.getIntegerArray(key)));
                    break;
                case StringArray:
                    mUIValueEdit.setText(Joiner.on(";").join(mStore.getStringArray(key)));
                    break;
                case ColorArray:
                    mUIValueEdit.setText(Joiner.on(";").join(mStore.getColorArray(key)));
                    break;
                case Color:
                    mUIValueEdit.setText(mStore.getColor(key).toString());
                    break;
                case Boolean:
                    mUIValueEdit.setText(Boolean.toString(mStore.getBoolean(key)));
                    break;
                case Integer:
                    mUIValueEdit.setText(Integer.toString(mStore.getInteger(key)));
                    break;
                case String:
                    //retrieve string entries from the native Store
                    mUIValueEdit.setText(mStore.getString(key));
                    break;
            }
        }
        //// Process any exception raised while retrieving data.
        catch (NotExistingKeyException eNotExistingKeyException) {
            displayMessage(eNotExistingKeyException.getMessage());
        } catch (InvalidTypeException eInvalidTypeException) {
            displayMessage(eInvalidTypeException.getMessage());
        }
    }

    private void displayMessage(String pMessage) {
        Toast.makeText(getActivity(), pMessage, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onSuccess(int pValue) {
        displayMessage(String.format(
                "Integer '%1$d' successfuly saved!", pValue));
    }

    @Override
    public void onSuccess(String pValue) {
        displayMessage(String.format(
                "String '%1$s' successfuly saved!", pValue));
    }

    @Override
    public void onSuccess(Color pValue) {
        displayMessage(String.format(
                "Color '%1$s' successfuly saved!", pValue));
    }

}
