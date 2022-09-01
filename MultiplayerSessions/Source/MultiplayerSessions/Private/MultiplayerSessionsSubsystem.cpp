// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() : 
createSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
findSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
joinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
startSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
destroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
    IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
    if (onlineSubsystem)
    {
        sessionInterface = onlineSubsystem->GetSessionInterface();
    }
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if (!sessionInterface.IsValid()) return;

    auto existingSession = sessionInterface->GetNamedSession(NAME_GameSession);
    if (existingSession != nullptr) { // if there exists a session
        bCreateSessionOnDestroy = true;
        lastNumPublicConnections = NumPublicConnections;
        lastMatchType = MatchType;
        
        DestroySession();
    }

    createSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(createSessionCompleteDelegate);
    
    lastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    lastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; //it's LAN match if not connected to any online subsystem
    lastSessionSettings->NumPublicConnections = NumPublicConnections;
    lastSessionSettings->bAllowJoinInProgress = true;
    lastSessionSettings->bAllowJoinViaPresence = true;
    lastSessionSettings->bShouldAdvertise = true;
    lastSessionSettings->bUsesPresence = true;
    lastSessionSettings->bUseLobbiesIfAvailable = true;
    lastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    lastSessionSettings->BuildUniqueId = 1;
    
    ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *lastSessionSettings))
    {
        sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(createSessionCompleteDelegateHandle);

        // Boardcast our own custom delegate
        multiplayerOnCreateSessionComplete.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
    if (!sessionInterface.IsValid()) return;
    
    findSessionsCompleteDelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(findSessionsCompleteDelegate);
    
    lastSessionSearch = MakeShareable(new FOnlineSessionSearch());
    lastSessionSearch->MaxSearchResults = MaxSearchResults;
    lastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
    lastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    
    ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), lastSessionSearch.ToSharedRef()))
    {
        sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(findSessionsCompleteDelegateHandle);
        multiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false); // an empty array
    }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
    if (!sessionInterface.IsValid()) 
    {
        multiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
        return;
    }

    findSessionsCompleteDelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(joinSessionCompleteDelegate);
    ULocalPlayer *localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
    {
        sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(joinSessionCompleteDelegateHandle);
        multiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    }
}

void UMultiplayerSessionsSubsystem::StartSession()
{

}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if (!sessionInterface.IsValid())
    {
        multiplayerOnDestroySessionComplete.Broadcast(false);
        return;
    }
    destroySessionCompleteDelegateHandle = sessionInterface->AddOnDestroySessionCompleteDelegate_Handle(destroySessionCompleteDelegate);
    if (!sessionInterface->DestroySession(NAME_GameSession)) // failed to Destroy the session
    {
        sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(destroySessionCompleteDelegateHandle);
        multiplayerOnDestroySessionComplete.Broadcast(false);
    }
    
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
    if (sessionInterface)
    {
        sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(createSessionCompleteDelegateHandle);
    }
    multiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (sessionInterface)
    {
        sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(findSessionsCompleteDelegateHandle);
    }
    if (lastSessionSearch->SearchResults.IsEmpty()) 
    {
        multiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
        return;
    }
    multiplayerOnFindSessionsComplete.Broadcast(lastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
    if (sessionInterface)
    {
        sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(joinSessionCompleteDelegateHandle);
    }
    multiplayerOnJoinSessionComplete.Broadcast(result);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{

}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
    if (sessionInterface)
    {
        sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(destroySessionCompleteDelegateHandle);
    }
    if (bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(lastNumPublicConnections, lastMatchType);
    }
    multiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
